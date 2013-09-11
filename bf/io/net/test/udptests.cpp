#include <map>
#include <gtest/gtest.h>

#include <openssl/sha.h>
#include <random>
#include <thread>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <bf/bf.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <condition_variable>
#include <atomic>
#include <sys/ioctl.h>
#include "bfsocket.h"
using namespace std;

namespace bitforge {

TEST(ServiceAddressTest, TestServiceAddress)
{
    int fd = (socket(AF_INET, SOCK_DGRAM, 0));
    NetworkInterfaceInfo::dumpInterfacesInfo(fd);
    close (fd);

    ServiceAddress test1("udp://127.0.0.1:8080/query");
    ASSERT_EQ(test1.protocol(), "udp");
    ASSERT_EQ(test1.port(), 8080);
    ASSERT_EQ(test1.host(), "127.0.0.1");
    ASSERT_EQ(test1.isMulticast(), false);
    ASSERT_EQ(test1.query(), "/query");
    
    ServiceAddress test2("http://234.1.1.1/query");
    ASSERT_EQ(test2.protocol(), "http");
    ASSERT_EQ(test2.port(), 80);
    ASSERT_EQ(test2.host(), "234.1.1.1");
    ASSERT_EQ(test2.isMulticast(), true);
    ASSERT_EQ(test2.query(), "/query");
    
    ServiceAddress test3("ftp://199.99.99.99");
    ASSERT_EQ(test3.protocol(), "ftp");
    ASSERT_EQ(test3.port(), 21);
    ASSERT_EQ(test3.host(), "199.99.99.99");
    ASSERT_EQ(test3.isMulticast(), false);
    ASSERT_EQ(test3.query().empty(), true);
    
    ServiceAddress test4("tcp://199.99.99.99:3141");
    ASSERT_EQ(test4.protocol(), "tcp");
    ASSERT_EQ(test4.port(), 3141);
    ASSERT_EQ(test4.host(), "199.99.99.99");
    ASSERT_EQ(test4.isMulticast(), false);
    ASSERT_EQ(test4.query().empty(), true);
}
    
class BFSocketTest : public ::testing::Test
{
public:

    virtual void SetUp()
    {
        m_doneSending = false;

        random_device r;
        m_port = (double)r() / (double)r.max() * 2000 + 7000;
        
        BUFFER_SIZE = (double)r() / (double)r.max() * 512 + 1350;
        
        m_inputData.open("/bin/gawk");
        
        SHA1_Init(&m_ctxSend);
        SHA1_Init(&m_ctxRecv);
    }
    
    virtual void createUDPSocket(string url, bool testSocketWO, string _bindDevice = string())
    {

        ServiceAddress serviceAddress(url, testSocketWO? ServiceAddress::stUDPRO : ServiceAddress::stUDPWO);
        m_bfSocket = BFSocketUPtr(new BFSocket(serviceAddress, _bindDevice));
        
        bzero(&m_streamAddr, sizeof(m_streamAddr));
        m_streamAddr.sin_family = AF_INET;
        m_streamAddr.sin_addr.s_addr = serviceAddress.hostAddress();
        m_streamAddr.sin_port = htons(m_port);
        
        m_sockFd = socket(AF_INET, SOCK_DGRAM, 0);
        if(m_sockFd == -1)
            THROW_SOCKET_EXCEPTION("Could not create socket - " << strerror(errno));
        
        if(!_bindDevice.empty())
        {
            if(setsockopt(m_sockFd, SOL_SOCKET, SO_BINDTODEVICE, _bindDevice.c_str(), _bindDevice.size()) == -1)
                THROW_SOCKET_EXCEPTION("Could not bind socket to device - " << strerror(errno));
        }
        
        if(!testSocketWO)
        {
            if(bind(m_sockFd, (struct sockaddr *) &m_streamAddr, sizeof(m_streamAddr)))
                THROW_SOCKET_EXCEPTION("Could not bind UDP Socket - " << strerror(errno));
            
            if(serviceAddress.isMulticast())
            {
                struct ip_mreq mreq;
                zero_init(mreq);
                mreq.imr_multiaddr.s_addr = serviceAddress.hostAddress();
                mreq.imr_interface.s_addr = htonl(INADDR_ANY);

                if (setsockopt(m_sockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
                    THROW_SOCKET_EXCEPTION("Could not add membership do socket - " << strerror(errno));
            }
        }
    }

    virtual void TearDown()
    {
        if(m_sockFd)
            close(m_sockFd);
    }
    
    virtual void checkSHAandSizes()
    {
        unsigned char shaSend[SHA_DIGEST_LENGTH], shaRecv[SHA_DIGEST_LENGTH];
        
        SHA1_Final(shaSend, &m_ctxSend);
        SHA1_Final(shaRecv, &m_ctxRecv);
        
        ASSERT_EQ(m_sentBytes, m_recvBytes);
        ASSERT_EQ(memcmp(shaSend, shaRecv, SHA_DIGEST_LENGTH), 0);
    }
    
protected:
    size_t BUFFER_SIZE;
    
    int m_port = 0;
    struct sockaddr_in m_streamAddr;
    BFSocketUPtr m_bfSocket;
    int m_sockFd = 0;
    
    ifstream m_inputData;
    
    std::atomic<bool> m_doneSending;

    size_t m_sentBytes = 0;
    size_t m_recvBytes = 0;
    SHA_CTX m_ctxSend;
    SHA_CTX m_ctxRecv;
    
    
public:
    ssize_t canRead()
    {
        int result = 0;
        if(ioctl(m_sockFd, FIONREAD, &result) == -1)
            THROW_SOCKET_EXCEPTION("Could not realize can read - " << strerror(errno));
        return result;
    };
};

TEST_F(BFSocketTest, TestUDPSocketRead)
{
    stringstream ss;
    ss << "udp://127.0.0.1:" << m_port;
    createUDPSocket(ss.str(), true);
    
    atomic<bool> doneSending(false);
    vector<char> v;
    {
        ifstream is("/bin/gawk");
        copy(istream_iterator<char>(is), istream_iterator<char>(), back_inserter(v));
    }
    auto f = [&]() {
        auto ptr = v.data();
        auto remain = v.size();
        while(remain)
        {
            auto size = min<int>(1024, remain);
            auto res = sendto(m_sockFd, ptr, size, 0, (struct sockaddr *)&m_streamAddr, sizeof(m_streamAddr));
            if(res == -1)
                throw ExceptionWithMessage(string("Could not send data ") + strerror(errno));
            ASSERT_EQ(res, size);
            remain -= res;
            ptr += res;
            usleep(1000);
        }
        doneSending = true;
    };
    
    char buff[v.size()];
    unsigned int received = 0;
    
    thread t(f);
    SHA_CTX shaCtx;
    SHA1_Init(&shaCtx);
    
    do
    {
        if(m_bfSocket->canRead()==0)
        {
            usleep(1);
            continue;
        }
        int res = m_bfSocket->read(&buff[received], v.size() - received);
        SHA1_Update(&shaCtx, &buff[received], res);
        received += res;
        ASSERT_LE(received, v.size());
        
    } while(!doneSending);
    
    t.join();
    unsigned char aux[SHA_DIGEST_LENGTH];
    unsigned char aux2[SHA_DIGEST_LENGTH];
    
    SHA1_Final(aux, &shaCtx);
    SHA1((unsigned char *)v.data(), v.size(), aux2);
    
    ASSERT_EQ(memcmp(aux, aux2, SHA_DIGEST_LENGTH), 0);
    ASSERT_EQ(v.size(), received);
    
}

TEST_F(BFSocketTest, TestUDPSocketWriteOf188Bytes)
{
    const int MSG_SIZE = 188*7;
    const int NUM_MSGS = 1000;
    
    ASSERT_GT(BUFFER_SIZE, MSG_SIZE);
    stringstream ss;
    ss << "udp://127.0.0.1:" << m_port;
    createUDPSocket(ss.str(), false);
    
    char sendBuff[MSG_SIZE];
    bzero(sendBuff, MSG_SIZE);

    sendBuff[0]  = 0x47;
    sendBuff[MSG_SIZE - 1]  = 0x48;
    
    int msgs = 0;
    
    thread t([&]()
    {
        for(int i = 0; i < NUM_MSGS; i++)
        {
            char recvBuff[BUFFER_SIZE];
            bzero(recvBuff, BUFFER_SIZE);

            auto received = recv(m_sockFd, recvBuff, BUFFER_SIZE, 0);
            ASSERT_EQ(received, MSG_SIZE);
            ASSERT_EQ(recvBuff[0], 0x47);
            ASSERT_EQ(recvBuff[received - 1], 0x48);
            msgs++;
        }
    });
    
    for(int i = 0; i < NUM_MSGS; i++)
    {
        m_bfSocket->write(sendBuff, MSG_SIZE);
        usleep(1);
    }
    
    t.join();
    ASSERT_EQ(msgs, NUM_MSGS);
}


TEST_F(BFSocketTest, TestUDPSocketReadOf188Bytes)
{
    const int MSG_SIZE = 188*7;
    const int NUM_MSGS = 1000;
    
    ASSERT_GT(BUFFER_SIZE, MSG_SIZE);
    stringstream ss;
    ss << "udp://127.0.0.1:" << m_port;
    createUDPSocket(ss.str(), true);
    char sendBuff[MSG_SIZE];
    zero_init(sendBuff);
    char recvBuff[BUFFER_SIZE];
    int msgs = 0;
    
    thread t([&]() {
        for(int i = 0; i < NUM_MSGS; i++)
        {
            auto received = m_bfSocket->read(recvBuff, BUFFER_SIZE);
            msgs++;
            ASSERT_EQ(received, MSG_SIZE);
        }
    });
    
    for(int i = 0; i < NUM_MSGS; i++)
    {
        if(sendto(m_sockFd, sendBuff, MSG_SIZE, 0, (struct sockaddr *)&m_streamAddr, sizeof(m_streamAddr)) == -1)
            throw ExceptionWithMessage(string("Could not send data ") + strerror(errno));
        usleep(1);
    }
    
    t.join();
    ASSERT_EQ(msgs, NUM_MSGS);
}

TEST_F(BFSocketTest, TestUDPSocketDeviceSelection)
{
    const int MSG_SIZE = 188*7;
    const int NUM_MSGS = 1000;
    
    ASSERT_GT(BUFFER_SIZE, MSG_SIZE);
    stringstream ss;
    ss << "udp://127.0.0.1:" << m_port;

    EXPECT_NO_THROW(createUDPSocket(ss.str(), true, "lo"));
    if (m_bfSocket)
    {
        char sendBuff[MSG_SIZE];
        zero_init(sendBuff);
        char recvBuff[BUFFER_SIZE];
        int msgs = 0;

        thread t([&]() {
            for(int i = 0; i < NUM_MSGS; i++)
            {
                auto received = m_bfSocket->read(recvBuff, BUFFER_SIZE);
                msgs++;
                ASSERT_EQ(received, MSG_SIZE);
            }
        });

        for(int i = 0; i < NUM_MSGS; i++)
        {
            if(sendto(m_sockFd, sendBuff, MSG_SIZE, 0, (struct sockaddr *)&m_streamAddr, sizeof(m_streamAddr)) == -1)
                throw ExceptionWithMessage(string("Could not send data ") + strerror(errno));
            usleep(1);
        }

        t.join();
        ASSERT_EQ(msgs, NUM_MSGS);
    }
    else
    {
        ASSERT_NE(getuid(), 0);
        std::cout << "WARNING: Unable to test Socket Device selection.  Not running as root user." << std::endl;
    }
}

TEST_F(BFSocketTest, TestUDPMulticastSocketRead)
{
    stringstream ss;
    ss << "udp://234.31.1.1:" << m_port;
    createUDPSocket(ss.str(), true);
    
    thread t([&]() 
    {
        while((!m_doneSending) || canRead())
        {
            char buffer[BUFFER_SIZE];
            auto res = m_bfSocket->read(buffer, BUFFER_SIZE);
            m_recvBytes += res;
            SHA1_Update(&m_ctxRecv, buffer, res);
        }
    });
    
    while(m_inputData.good() && !m_inputData.eof())
    {
        char buffer[BUFFER_SIZE];
        m_inputData.read(buffer, BUFFER_SIZE);
        auto read = m_inputData.gcount();
        
        auto res = sendto(m_sockFd, buffer, read, 0, (struct sockaddr *)&m_streamAddr, sizeof(m_streamAddr));

        if (m_inputData.eof() || !m_inputData.good()) m_doneSending = true;

        if(res == -1)
            throw ExceptionWithMessage(string("Could not send data ") + strerror(errno));

        m_sentBytes += res;
        SHA1_Update(&m_ctxSend, buffer, read);
    }
    
    m_doneSending = true;
    
    t.join();
    
    ASSERT_GT(m_recvBytes, 0);
}

TEST_F(BFSocketTest, TestUDPMulticastSocketWrite)
{    
    stringstream ss;
    ss << "udp://234.31.1.1:" << m_port;
    bool testSocketWO = false;
    createUDPSocket(ss.str(), testSocketWO);
    thread t([&]() 
    {
        while((!m_doneSending) || canRead())
        {
            char buffer[BUFFER_SIZE];
            auto res = recv(m_sockFd, buffer, BUFFER_SIZE, 0);
            if(res == -1)
                THROW_SOCKET_EXCEPTION("Could not recv form multicast Socket - " << strerror(errno));
            m_recvBytes += res;
            SHA1_Update(&m_ctxRecv, buffer, res);
        }
    });
    
    while(m_inputData.good() && !m_inputData.eof())
    {
        char buffer[BUFFER_SIZE];
        m_inputData.read(buffer, BUFFER_SIZE);
        auto res = m_bfSocket->write(buffer, m_inputData.gcount());

        if (m_inputData.eof() || !m_inputData.good()) m_doneSending = true;

        m_sentBytes += res;
        SHA1_Update(&m_ctxSend, buffer, m_inputData.gcount());
    }

    t.join();

    checkSHAandSizes();
}

TEST_F(BFSocketTest, TestUDPReadNonblocking)
{    
    stringstream ss;
    ss << "udp://127.0.0.1:" << m_port;
    bool testSocketWO = true;
    createUDPSocket(ss.str(), testSocketWO);
    
    ASSERT_EQ(m_bfSocket->isNonblocking(), false);
    m_bfSocket->setNonblocking();
    ASSERT_EQ(m_bfSocket->isNonblocking(), true);
    
    char buffer[BUFFER_SIZE];
    auto rec = m_bfSocket->read(buffer, BUFFER_SIZE);
    ASSERT_EQ(rec, 0);
}

TEST_F(BFSocketTest, TestUDPWriteNonblocking)
{    
    stringstream ss;
    ss << "udp://127.0.0.1:" << m_port;
    bool testSocketWO = true;
    createUDPSocket(ss.str(), testSocketWO);
    m_bfSocket->setNonblocking();

    char buffer[BUFFER_SIZE];
    for(unsigned int i = 0; i != BUFFER_SIZE; i++)
        buffer[i] = i;

    auto rec = m_bfSocket->write(buffer, BUFFER_SIZE);
    ASSERT_GT(rec, 0);

    rec = m_bfSocket->write(buffer, BUFFER_SIZE);
    ASSERT_GT(rec, 0);

    rec = m_bfSocket->write(buffer, BUFFER_SIZE);
    ASSERT_GT(rec, 0);
}

TEST_F(BFSocketTest, TestUDPMulticastTTL)
{    
    stringstream ss;
    ss << "udp://127.0.0.1:" << m_port;
    bool testSocketWO = true;
    createUDPSocket(ss.str(), testSocketWO);
    m_bfSocket->setMulticastTTL(2);
    
    int ttl = 0;
    auto size = sizeof(ttl);
    getsockopt(m_bfSocket->fileDescriptor().get(), IPPROTO_IP, IP_MULTICAST_TTL, &ttl, (socklen_t*)&size);
    
    ASSERT_EQ(2, ttl);
    
    m_bfSocket->setMulticastTTL(4);
    getsockopt(m_bfSocket->fileDescriptor().get(), IPPROTO_IP, IP_MULTICAST_TTL, &ttl, (socklen_t*)&size);
    
    ASSERT_EQ(4, ttl);
}

}
