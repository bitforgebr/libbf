#include <map>
#include <gtest/gtest.h>

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
#include "bfsocket.h"

using namespace std;

namespace bitforge {
    
class BFTCPSocketTest : public ::testing::Test
{
public:

    virtual void SetUp()
    {
        random_device r;
        m_port = (double)r() / (double)r.max() * 2000 + 7000;
    }
    
    virtual void createTCPSocket(string _url, bool _isTestingServer)
    {
        ServiceAddress serviceAddress(_url, _isTestingServer? ServiceAddress::stTCPServer : ServiceAddress::stTCP);
        
        bzero(&m_streamAddr, sizeof(m_streamAddr));
        m_streamAddr.sin_family = AF_INET;
        m_streamAddr.sin_addr.s_addr = serviceAddress.hostAddress();
        m_streamAddr.sin_port = htons(m_port);
        
        m_sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(m_sockFd == -1)
            THROW_SOCKET_EXCEPTION("Could not create socket - " << strerror(errno));
        
        if(!_isTestingServer)
        {
            if(bind(m_sockFd, (struct sockaddr *) &m_streamAddr, sizeof(m_streamAddr)) == -1)
                THROW_SOCKET_EXCEPTION("Could not connect TCP Socket - " << strerror(errno));
        
            if(listen(m_sockFd, 2) == -1)
                THROW_SOCKET_EXCEPTION("Could not listen TCP Socket - " << strerror(errno));
            
            thread t([&](){
                m_clientFd = accept(m_sockFd, nullptr, 0);
                if(m_clientFd == -1)
                    THROW_SOCKET_EXCEPTION("Could not accept TCP connection - " << strerror(errno));
            });
            
            m_bfSocket = BFSocketUPtr(new BFSocket(serviceAddress));
            t.join();
        }
        else
        {
            m_bfSocket = BFSocketUPtr(new BFSocket(serviceAddress));
        }
    }

    virtual void TearDown()
    {
        if(m_sockFd)
            close(m_sockFd);
    }
    
protected:
    
    int m_port;
    struct sockaddr_in m_streamAddr;
    BFSocketUPtr m_bfSocket;
    int m_sockFd = 0;
    int m_clientFd;
};

TEST_F(BFTCPSocketTest, TestTCPSocketWrite)
{
    stringstream ss;
    ss << "tcp://127.0.0.1:" << m_port;
    bool isTestingServer = false;
    createTCPSocket(ss.str(), isTestingServer);
    
    vector<char> v;
    {
        ifstream is("/bin/gawk");
        copy(istream_iterator<char>(is), istream_iterator<char>(), back_inserter(v));
    }
    
    unsigned int received = 0;
    char buff[v.size()];
    thread t([&]() {
        while(received < v.size())
        {
            auto res = recv(m_clientFd, &buff[received], v.size() - received, 0);
            if(res == -1)
                THROW_SOCKET_EXCEPTION("Could not recv form multicast Socket - " << strerror(errno));
            received += res;
        }
    });
    
    auto ptr = v.data();
    auto remain = v.size();
    while(remain)
    {
        auto size = min<int>(1024, remain);
        auto res = m_bfSocket->write(ptr, size);
        remain -= res;
        ptr += res;
    }
    
    t.join();
    ASSERT_EQ(v.size(), received);
    ASSERT_EQ(memcmp(buff, v.data(), v.size()), 0);
}

TEST_F(BFTCPSocketTest, TestTCPSocketSpliceWrite)
{
    stringstream ss;
    ss << "tcp://127.0.0.1:" << m_port;
    bool isTestingServer = false;
    createTCPSocket(ss.str(), isTestingServer);
    
    vector<char> v;
    {
        ifstream is("/bin/gawk");
        copy(istream_iterator<char>(is), istream_iterator<char>(), back_inserter(v));
    }
    
    unsigned int received = 0;
    char buff[v.size()];

    thread t([&]() {
        while(received < v.size())
        {
            auto res = recv(m_clientFd, &buff[received], v.size() - received, 0);
            if(res == -1)
                THROW_SOCKET_EXCEPTION("Could not recv form multicast Socket - " << strerror(errno));
            received += res;
        }
    });
    
    int pipeFd[2];
    pipe(pipeFd);
    
    try
    {
        auto ptr = v.data();
        auto remain = v.size();
        while(remain)
        {
            auto size = min<int>(1024, remain);
            size = write(pipeFd[1], ptr, size);
            auto res = m_bfSocket->spliceWrite(pipeFd[0], size);
            remain -= res;
            ptr += res;
        }
    }
    catch(SocketException e)
    {
        std::cerr << "Socket exception: " << e.what() << std::endl;
    }

    t.join();

    ASSERT_EQ(v.size(), received);
    ASSERT_EQ(memcmp(buff, v.data(), v.size()), 0);
    close(pipeFd[0]);
    close(pipeFd[1]);
}

TEST_F(BFTCPSocketTest, TestTCPSocketRead)
{
    stringstream ss;
    ss << "tcp://127.0.0.1:" << m_port;
    bool isTestingServer = false;
    createTCPSocket(ss.str(), isTestingServer);
    
    vector<char> v;
    {
        ifstream is("/bin/gawk");
        copy(istream_iterator<char>(is), istream_iterator<char>(), back_inserter(v));
    }
    
    unsigned int received = 0;
    char buff[v.size()];
    thread t([&]() {
        while(received < v.size())
            received += m_bfSocket->read(&buff[received], v.size() - received);
    });
    
    auto ptr = v.data();
    auto remain = v.size();
    while(remain)
    {
        auto size = min<int>(1024, remain);
        auto res = send(m_clientFd, ptr, size, 0);
        remain -= res;
        ptr += res;
    }
    
    t.join();
    ASSERT_EQ(v.size(), received);
    ASSERT_EQ(memcmp(buff, v.data(), v.size()), 0);
}

TEST_F(BFTCPSocketTest, TestTCPServerSocket)
{
    stringstream ss;
    ss << "tcp://127.0.0.1:" << m_port;
    bool isTestingServer = true;
    createTCPSocket(ss.str(), isTestingServer);
    
    thread t([&]() {
        BFSocketPtr bfSocket = m_bfSocket->acceptClient();
        ASSERT_EQ(bfSocket->serviceAddress().socketType(), ServiceAddress::stTCP);
    });
    
    if(connect(m_sockFd, (struct sockaddr *) &m_streamAddr, sizeof(m_streamAddr)) == -1)
        THROW_SOCKET_EXCEPTION("Could not recv form multicast Socket - " << strerror(errno));
    
    t.join();
}

TEST_F(BFTCPSocketTest, TestThrowsExceptionOnInvalidAccept)
{
    stringstream ss;
    ss << "udp://127.0.0.1:" << m_port;
    ServiceAddress addr(ss.str(), ServiceAddress::stUDP);
    m_bfSocket = BFSocketUPtr(new BFSocket(addr));
    ASSERT_THROW(m_bfSocket->acceptClient(), SocketException);
}

}
