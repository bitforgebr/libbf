#include <gtest/gtest.h>
#include "bfsocket.h"
#include <ev++.h>
#include <random>
#include <atomic>
#include <boost/concept_check.hpp>
#include <fstream>
#include <thread>

using namespace std;

namespace bitforge {

typedef struct ev_loop EvLoop;
class BFSocketLibEVTest : public ::testing::Test
{
public:

    virtual void SetUp()
    {
        random_device r;
        m_sendBufferSize = (double)r() / (double)r.max() * 512 + 1300;
        m_receiveBufferSize = max(m_sendBufferSize, (size_t)((double)r() / (double)r.max() * 512 + 1300));
        m_port = (double)r() / (double)r.max() * 2000 + 7000;
        m_evLoop = EV_DEFAULT;
        m_doneSending = false;
    }
    
    virtual void createUDPSockets(string _url, bool _testSocketWO)
    {
        ServiceAddress serviceAddress(_url, _testSocketWO? ServiceAddress::stUDPRO : ServiceAddress::stUDPWO);
        m_bfSocket = BFSocketUPtr(new BFSocket(serviceAddress));
        
        bzero(&m_streamAddr, sizeof(m_streamAddr));
        m_streamAddr.sin_family = AF_INET;
        m_streamAddr.sin_addr.s_addr = serviceAddress.hostAddress();
        m_streamAddr.sin_port = htons(m_port);
        
        m_sockFd = socket(AF_INET, SOCK_DGRAM, 0);
        if(m_sockFd == -1)
            THROW_SOCKET_EXCEPTION("Could not create socket - " << strerror(errno));
        
        if(!_testSocketWO)
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
    
    void readCallback(ev::io& w, int)
    {
        char buf[m_receiveBufferSize];
        
        auto res = m_bfSocket->read(buf, m_receiveBufferSize);
        m_receivedBytes += res;
        ASSERT_EQ(buf[0], 0x47);
        ASSERT_EQ(res, m_messageSize);
        
        if(m_doneSending)
            w.stop();
    }

    virtual void TearDown()
    {
        if(m_sockFd)
            close(m_sockFd);
    }
    
protected:
    
    EvLoop *m_evLoop;
    int m_port;
    struct sockaddr_in m_streamAddr;
    BFSocketUPtr m_bfSocket;
    int m_sockFd = 0;
    
    size_t m_messageSize;
    size_t m_sendBufferSize;
    size_t m_receiveBufferSize;
    atomic<size_t> m_sentBytes;
    size_t m_receivedBytes;
    atomic<bool> m_doneSending;
};

TEST_F(BFSocketLibEVTest, UDPSocket)
{
    stringstream ss;
    ss << "udp://127.0.0.1:" << m_port;
    bool testSocketWO = true;
    createUDPSockets(ss.str(), testSocketWO);
    m_messageSize = 188;
    char data[m_messageSize*100];
    auto end = data + m_messageSize*100;
    auto ptr = data;
    
    ev::io bfSocketReadable;
    bfSocketReadable.set<BFSocketLibEVTest, &BFSocketLibEVTest::readCallback>(this);
    bfSocketReadable.start(m_bfSocket->fileDescriptor() .get(), ev::READ);
    
    thread t([&]() {
       ev_run(m_evLoop, 0);
    });
    
    while(ptr != end)
    {
        *ptr = 0x47;
        auto res = sendto(m_sockFd, ptr, m_messageSize, 0, (struct sockaddr *) &m_streamAddr, sizeof(m_streamAddr));
        ptr += res;
        m_sentBytes += res;
    }
    m_doneSending = true;
    t.join();
}
}
