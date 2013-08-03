#ifndef BFSOCKET_H
#define BFSOCKET_H

#include "../bf.h"
#include "bfio.h"

#include <arpa/inet.h>

#include <net/if.h>
#include <vector>

namespace bitforge {

#ifndef NDEBUG
#define THROW_SOCKET_EXCEPTION(STR)                             \
    do { ::std::stringstream ss;                                \
        ss << basename(__FILE__) << ':' << __LINE__ << ':' << __PRETTY_FUNCTION__ <<  \
        STR; throw SocketException(ss.str(), errno);            \
    } while (false)

#define THROW_SOCKET_ADDR_EXCEPTION(STR)                        \
    do { ::std::stringstream ss;                                \
        ss << basename(__FILE__) << ':' << __LINE__ << ':' << __PRETTY_FUNCTION__ <<  \
        STR; throw SocketAddrerssException(ss.str());            \
    } while (false)

#else
#define THROW_SOCKET_EXCEPTION(STR) do { ::std::stringstream ss; ss << STR; throw SocketException(ss.str(), errno); } while (false)
#define THROW_SOCKET_ADDR_EXCEPTION(STR) do { ::std::stringstream ss; ss << STR; throw SocketAddrerssException(ss.str()); } while (false)
#endif

class SocketException: public IOException
{
public:
    SocketException(int error_no): IOException(error_no) {};
    SocketException(std::string err_msg, int error_no): IOException(err_msg, error_no) {};
};

class SocketAddrerssException: public SocketException
{
public:
    SocketAddrerssException(std::string err_msg): SocketException(err_msg, 0) {};
};

class NetworkInterfaceInfo
{
protected:
    NetworkInterfaceInfo(NCString name);

    NCString    m_name;
    short       m_flags = 0;
    struct sockaddr m_ifAddr;
    struct sockaddr m_p2pDstAddr;
    struct sockaddr m_broadCastAddr;
    struct sockaddr m_netMask;
    struct sockaddr m_hwAddr;
    int         m_metric = 0;
    int         m_mtu = 0;
    int         m_txQueueLen = 0;

public:
    static void dumpInterfacesInfo(int fd);
    static std::vector<std::string> listInterfaces(int fd);
    static NetworkInterfaceInfo getInfo(int fd, std::string iface);

    NCString name() const { return m_name; }
    
    bool isUp() const                 { return m_flags & IFF_UP; }            // Interface is running
    bool hasBroadcast() const         { return m_flags & IFF_BROADCAST; }     // Valid broadcast address set.
    bool isLoopback() const           { return m_flags & IFF_LOOPBACK; }      // Interface is a loopback interface.
    bool isP2P() const                { return m_flags & IFF_POINTOPOINT; }   // Interface is a point-to-point link.
    bool isRunning() const            { return m_flags & IFF_RUNNING; }       // Resources allocated.
    bool noArp() const                { return m_flags & IFF_NOARP; }         // No arp protocol, L2 destination address not set.
    bool isInPromicMode() const       { return m_flags & IFF_PROMISC; }       // Interface is in promiscuous mode.
    bool noTrailers() const           { return m_flags & IFF_NOTRAILERS; }    // Avoid use of trailers.
    bool allMulti() const             { return m_flags & IFF_ALLMULTI; }      // Receive all multicast packets.
    bool isMasterOfBalance() const    { return m_flags & IFF_MASTER; }        // Master of a load balancing bundle.
    bool isSlaveOfBalance() const     { return m_flags & IFF_SLAVE; }         // Slave of a load balancing bundle.
    bool supportMulticast() const     { return m_flags & IFF_MULTICAST; }     // Supports multicast
    bool canSelectMedia() const       { return m_flags & IFF_PORTSEL; }       // Is able to select media type via ifmap.
    bool autoMediaSelect() const      { return m_flags & IFF_AUTOMEDIA; }     // Auto media selection active.
    bool dynamicAddr() const          { return m_flags & IFF_DYNAMIC; }       // The addresses are lost when the interface goes down.
#ifdef IFF_LOWER_UP
    bool driverSignalsL1() const      { return m_flags & IFF_LOWER_UP; }      // Driver signals L1 up (since Linux 2.6.17)
#endif
#ifdef IFF_DORMANT
    bool driverSignalsDormant() const { return m_flags & IFF_DORMANT; }       // Driver signals dormant (since Linux 2.6.17)
#endif
#ifdef IFF_ECHO
    bool echoPackets() const          { return m_flags & IFF_ECHO; }          // Echo sent packets (since Linux 2.6.25)
#endif

    const struct sockaddr *ifAddr() const { return &m_ifAddr; };
    const struct sockaddr *p2pDstAddr() const { return &m_p2pDstAddr; };
    const struct sockaddr *broadCastAddr() const { return &m_broadCastAddr; };
    const struct sockaddr *netMask() const { return &m_netMask; };
    const struct sockaddr *hwAddr() const { return &m_hwAddr; };
    int metric() const { return m_metric; };
    int mtu() const { return m_mtu; };
    int txQueueLen() const { return m_txQueueLen; };
};

class ServiceAddress {
public:
    typedef struct sockaddr_in SockAddrIn;
    enum SocketType
    {
        stUNKNOWN,
        stRAW,
        stUDP,
        stUDPRO,
        stUDPWO,
        stTCP,
        stTCPServer,
        stSCTP
    };
    
    enum ProtocolType
    {
        ptUNKNOWN,
        ptRAW,
        ptHTTP,
        ptHTTPS
    };
    
    ServiceAddress() {}
    ServiceAddress(std::string _url, SocketType _type = stUNKNOWN);

private:
    SocketType  m_socketType = stUNKNOWN;
    ProtocolType  m_protocolType = ptUNKNOWN;
    NCString    m_url;
    NCString    m_protocol;
    NCString    m_query;
    NCString    m_host;
    SockAddrIn  m_sockAddr;

public:
    SocketType  socketType() const { return m_socketType; }
    ProtocolType  protocolType() const { return m_protocolType; }
    NCString    url() const        { return m_url; }
    NCString    protocol() const   { return m_protocol; }
    NCString    query() const      { return m_query; }
    NCString    host() const       { return m_host; }
    in_addr_t   hostAddress() const{ return m_sockAddr.sin_addr.s_addr; }
    short       port() const       { return ntohs(m_sockAddr.sin_port); }
    bool        isMulticast() const{ return IN_MULTICAST(htonl(m_sockAddr.sin_addr.s_addr)); };
    const SockAddrIn* sockAddr()   { return &m_sockAddr; }
};

class BFSocket: public BFSimpleFd
{
public:
    explicit BFSocket(ServiceAddress _addr, std::string _bindDevice = std::string());
    virtual ~BFSocket();

private:
    explicit BFSocket(ServiceAddress _addr, FileDescriptor&& _fd);

    ServiceAddress  m_addr;
    int             m_sendBufferSize;
    bool            m_isNonblocking;
    typedef StrongTypedef<int> SocketFlags;
    SocketFlags     m_defaultSendFlags;
    std::string m_device;
    
public:
    virtual void setMulticastTTL(int _TTL);
    
    virtual ssize_t read(void* _buffer, size_t _size);
    virtual ssize_t write(void* _buffer, size_t _size);
    virtual ssize_t spliceWrite(int pipeFd, size_t);
    
    virtual ssize_t canRead();
    virtual ssize_t canWrite();

    virtual std::shared_ptr<BFSocket> acceptClient();
    
    virtual ServiceAddress serviceAddress() { return m_addr; };
    virtual const FileDescriptor fileDescriptor() const { return m_fd; };
    virtual void setNonblocking();
    virtual bool isNonblocking() { return m_isNonblocking; };
    virtual std::string device() { return m_device; };
};
typedef std::unique_ptr<BFSocket> BFSocketUPtr;
typedef std::shared_ptr<BFSocket> BFSocketPtr;

}
#endif // BFSOCKET_H
