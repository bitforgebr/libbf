#include "bfsocket.h"
#include <signal.h>
#include <sstream>

#include <sys/socket.h>

#include <sys/ioctl.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

namespace bitforge {

NetworkInterfaceInfo::NetworkInterfaceInfo ( NCString name ):
    m_name(name)
{
    zero_init(m_ifAddr);
    zero_init(m_p2pDstAddr);
    zero_init(m_broadCastAddr);
    zero_init(m_netMask);
    zero_init(m_hwAddr);
}

NetworkInterfaceInfo NetworkInterfaceInfo::getInfo(int fd, std::string iface)
{
    NetworkInterfaceInfo result(iface);

    struct ifreq ifreq;
    zero_init(ifreq);

    strncpy(ifreq.ifr_name, iface.c_str(), IFNAMSIZ);

#define GET_PROP(FLAG, CLASS, STRUCT) \
    do { if (ioctl(fd, FLAG, &ifreq) != -1) result.CLASS = ifreq.STRUCT; } while(false)

    GET_PROP(SIOCGIFFLAGS,   m_flags,         ifr_flags);
    GET_PROP(SIOCGIFADDR,    m_ifAddr,        ifr_addr);
    GET_PROP(SIOCGIFDSTADDR, m_p2pDstAddr,    ifr_dstaddr);
    GET_PROP(SIOCGIFBRDADDR, m_broadCastAddr, ifr_broadaddr);
    GET_PROP(SIOCGIFNETMASK, m_netMask,       ifr_netmask);
    GET_PROP(SIOCGIFMETRIC,  m_metric,        ifr_metric);
    GET_PROP(SIOCGIFMTU,     m_mtu,           ifr_mtu);
    GET_PROP(SIOCGIFHWADDR,  m_hwAddr,        ifr_hwaddr);
    GET_PROP(SIOCGIFTXQLEN,  m_txQueueLen,    ifr_qlen);

    return result;
}

std::vector<std::string> NetworkInterfaceInfo::listInterfaces(int fd)
{
    std::vector<std::string> result;

    struct ifreq ifreq;
    zero_init(ifreq);

    ifreq.ifr_ifindex = 1;

    int ret;
    while(true)
    {
        ret = ioctl(fd, SIOCGIFNAME, &ifreq);
        if (ret != -1)
            result.push_back(ifreq.ifr_name);
        else
            break;

        ifreq.ifr_ifindex++;
    }

    return result;
}

void NetworkInterfaceInfo::dumpInterfacesInfo(int fd)
{
    for(auto iface : NetworkInterfaceInfo::listInterfaces(fd))
    {
        auto info = NetworkInterfaceInfo::getInfo(fd, iface);

        std::cout << iface << "\n"
            << "\tIs Up: " << (info.isUp() ? "Y" : "N") << "\n"
            << "\tHas broadcast address: " << (info.hasBroadcast() ? "Y" : "N") << "\n"
            << "\tIs Loopback: " << (info.isLoopback() ? "Y" : "N") << "\n"
            << "\tIs point-to-point: " << (info.isP2P() ? "Y" : "N") << "\n"
            << "\tResources allocated: " << (info.isRunning() ? "Y" : "N") << "\n"
            << "\tNo arp protocol: " << (info.noArp() ? "T" : "F") << "\n"
            << "\tIs in promic. mode: " << (info.isInPromicMode() ? "Y" : "N") << "\n"
            << "\tAvoid use of trailers: " << (info.noTrailers() ? "Y" : "N") << "\n"
            << "\tReceive all multicast packets: " << (info.allMulti() ? "Y" : "N") << "\n"
            << "\tReceive all multicast packets.: " << (info.isMasterOfBalance() ? "Y" : "N") << "\n"
            << "\tSlave of a load balancing bundle: " << (info.isSlaveOfBalance() ? "Y" : "N") << "\n"
            << "\tSupports multicast: " << (info.supportMulticast() ? "Y" : "N") << "\n"
            << "\tIs able to select media type via ifmap: " << (info.canSelectMedia() ? "Y" : "N") << "\n"
            << "\tAuto media selection active: " << (info.autoMediaSelect() ? "Y" : "N") << "\n"
            << "\tThe addresses are lost when the interface goes down: " << (info.dynamicAddr() ? "Y" : "N") << "\n"
#ifdef IFF_LOWER_UP
            << "\tDriver signals L1 up: " << info.driverSignalsL1()  << "\n"
#endif
#ifdef IFF_DORMANT
            << "\tDriver signals dormant: " << info.driverSignalsDormant()  << "\n"
#endif
#ifdef IFF_ECHO
            << "\tEcho sent packets: " << info.echoPackets()  << "\n"
#endif
            << "\tAddr: " << inet_ntoa(((struct sockaddr_in*)info.ifAddr())->sin_addr) << "\n"
            << "\tP-to-P dest Addr: " << inet_ntoa(((struct sockaddr_in*)info.p2pDstAddr())->sin_addr) << "\n"
            << "\tBroadcast address: " << inet_ntoa(((struct sockaddr_in*)info.broadCastAddr())->sin_addr) << "\n"
            << "\tNetmask: " << inet_ntoa(((struct sockaddr_in*)info.netMask())->sin_addr) << "\n"
            << "\tMAC Addr: " << inet_ntoa(((struct sockaddr_in*)info.hwAddr())->sin_addr) << "\n"
            << "\tMetric: " << info.metric()  << "\n"
            << "\tMTU: " << info.mtu()  << "\n"
            << "\tTX Queue length: " << info.txQueueLen()  << "\n";
    }

    std::cout << std::flush;
}

ServiceAddress::ServiceAddress(std::string _url, ServiceAddress::SocketType _type)
{
    m_url = _url;
    zero_init(m_sockAddr);
    m_sockAddr.sin_family = AF_INET;

    auto protEnd = _url.find("://");
    if (protEnd == std::string::npos)
        THROW_SOCKET_ADDR_EXCEPTION("Missing protocol in '" << _url << "'");

    m_protocol = _url.substr(0,protEnd);

    std::string hostPort = _url.substr(protEnd + 3);

    auto hostEnd = hostPort.find(":");
    if (hostEnd == std::string::npos)
    {
        struct servent* servent = getservbyname(m_protocol.c_str(), nullptr);
        if (!servent)
            THROW_SOCKET_ADDR_EXCEPTION("Missing/unable to deduce port in '" << _url << "'");

        m_sockAddr.sin_port = servent->s_port;

        hostEnd = hostPort.find("/");
        if (hostEnd == std::string::npos)
            m_host = hostPort;
        else
        {
            m_host = hostPort.substr(0, hostEnd);
            m_query = hostPort.substr(hostEnd);
        }
    }
    else
    {
        m_host = hostPort.substr(0, hostEnd);

        auto portEnd = hostPort.find("/");
        if (portEnd == std::string::npos)
            m_sockAddr.sin_port = htons(atoi(hostPort.substr(hostEnd+1).c_str()));
        else
        {
            m_sockAddr.sin_port = htons(atoi(hostPort.substr(hostEnd+1, portEnd).c_str()));
            m_query = hostPort.substr(portEnd);
        }
    }

    if (m_host.empty())
        THROW_SOCKET_ADDR_EXCEPTION("Missing host in '" << _url << "'");

    m_sockAddr.sin_addr.s_addr = inet_addr(m_host.c_str());

    // If not set, try to guess
    if (_type == stUNKNOWN && !m_protocol.empty())
    {
        char c = tolower(m_protocol[0]);
        switch(c)
        {
            case 'h':
                if (ICompStr(m_protocol, "http"))
                {
                    m_protocolType = ptHTTP;
                    m_socketType = stTCP;
                    goto DONE;
                }
                else if (ICompStr(m_protocol, "https"))
                {
                    m_protocolType = ptHTTPS;
                    m_socketType = stTCP;
                    goto DONE;
                }
                break;

            case 't':
                if (ICompStr(m_protocol, "tcp"))
                {
                    m_socketType = stTCP;
                    goto DONE;
                }
                break;

            case 'u':
                if (ICompStr(m_protocol, "udp"))
                {
                    m_socketType = stUDP;
                    goto DONE;
                }
                break;
        }

        m_socketType = _type;

        DONE:
        {}
    }
    else
        m_socketType = _type;
};

BFSocket::BFSocket(ServiceAddress _addr, FileDescriptor&& _fd):
    BFSimpleFd(_fd),
    m_addr(_addr)
{
}

BFSocket::BFSocket(ServiceAddress _addr, std::string _bindDevice):
    m_addr(_addr),
    m_isNonblocking(false)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = m_addr.hostAddress();
    addr.sin_port = htons(m_addr.port());

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        THROW_SOCKET_EXCEPTION("Unable to set sigpipe handler - "  << strerror(errno));
    
    switch(m_addr.socketType())
    {
        case ServiceAddress::stUDP:
        case ServiceAddress::stUDPRO:
        case ServiceAddress::stUDPWO:
        {
            m_fd.set(socket(AF_INET, SOCK_DGRAM, 0));
            if (m_fd.get() == -1)
                THROW_SOCKET_EXCEPTION("Unable to create socket - "  << strerror(errno));
            
            if(!_bindDevice.empty())
            {
                m_device = _bindDevice;
                if(setsockopt(m_fd.get(), SOL_SOCKET, SO_BINDTODEVICE, _bindDevice.c_str(), _bindDevice.size()) == -1)
                    THROW_SOCKET_EXCEPTION("Unable to bind socket to device - "  << strerror(errno));
                
            }
            
            int i = 1;
            if(setsockopt(m_fd.get(), SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)))
                THROW_SOCKET_EXCEPTION("Unable to set socket reuse addr - "  << strerror(errno));
            
            if(m_addr.socketType() != ServiceAddress::stUDPWO)
            {
                if(bind(m_fd.get(), (struct sockaddr *) &addr, sizeof(addr)) == 1)
                    THROW_SOCKET_EXCEPTION("Could not bind UDP Socket - " << strerror(errno));

                if(m_addr.isMulticast())
                {
                    struct ip_mreq mreq;
                    zero_init(mreq);
                    mreq.imr_multiaddr.s_addr = m_addr.hostAddress();
                    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

                    if (setsockopt(m_fd.get(), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
                        THROW_SOCKET_EXCEPTION("Could not add membership do socket - " << strerror(errno));
                }
            }

            m_defaultSendFlags.set(0);
            break;
        }
        case ServiceAddress::stTCP:
        case ServiceAddress::stTCPServer:
        {
            m_fd.set(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
            if (m_fd.get() == -1)
                THROW_SOCKET_EXCEPTION("Unable to create socket - "  << strerror(errno));
            
            if(!_bindDevice.empty())
            {
                m_device = _bindDevice;
                if(setsockopt(m_fd.get(), SOL_SOCKET, SO_BINDTODEVICE, _bindDevice.c_str(), _bindDevice.size()) == 1)
                    THROW_SOCKET_EXCEPTION("Unable to bind socket to device - "  << strerror(errno));
            }

            if(m_addr.socketType() == ServiceAddress::stTCP)
            {
                if(connect(m_fd.get(), (struct sockaddr *) &addr, sizeof(addr)) == -1)
                    THROW_SOCKET_EXCEPTION("Could not connect TCP Socket - " << strerror(errno));
            }
            else
            {
                if(bind(m_fd.get(), (struct sockaddr *) &addr, sizeof(addr)) == -1)
                    THROW_SOCKET_EXCEPTION("Could not connect TCP Socket - " << strerror(errno));
        
                if(listen(m_fd.get(), 2) == -1) // TODO: Number of connection queue should be given
                    THROW_SOCKET_EXCEPTION("Could not listen TCP Socket - " << strerror(errno));
            }
            m_defaultSendFlags.set(MSG_MORE);
            break;
        }
        default:
        {
            THROW_SOCKET_EXCEPTION("Socket type unknown: " << m_addr.socketType());
        }
    }

    socklen_t len = sizeof(m_sendBufferSize);
    if(getsockopt(m_fd.get(), SOL_SOCKET, SO_SNDBUF, &m_sendBufferSize, &len) == -1)
        THROW_SOCKET_EXCEPTION("Could not get send buffer size - " << strerror(errno));
};

ssize_t BFSocket::read(void* _buffer, size_t _size)
{
    auto res = recv(m_fd.get(), _buffer, _size, 0);
    if(res == -1)
    {
        switch(errno)
        {
            case EAGAIN:
                return 0;
            default:
                THROW_SOCKET_EXCEPTION("Could not receive data from Socket - " << strerror(errno));
        }
    }
    return res;
};

ssize_t BFSocket::write(void* _buffer, size_t _size)
{
    ssize_t res;
    if(m_addr.socketType() == ServiceAddress::stTCP)
        res = send(m_fd.get(), _buffer, _size, m_defaultSendFlags.get());
    else
        res = sendto(m_fd.get(), _buffer, _size, m_defaultSendFlags.get(), (struct sockaddr *) m_addr.sockAddr(), sizeof(*m_addr.sockAddr()));

    if(res == -1)
        THROW_SOCKET_EXCEPTION("Could not write data to Socket - " << strerror(errno));
    return res;
};

ssize_t BFSocket::spliceWrite(int pipeFd, size_t _size)
{
    ssize_t res;
    switch(m_addr.socketType())
    {
        case ServiceAddress::stUDP:
        case ServiceAddress::stUDPRO:
        case ServiceAddress::stUDPWO:
        {
            THROW_SOCKET_EXCEPTION("We can't splice to udp sockets");
            break;
        }
        case ServiceAddress::stTCP:
        {
            if(m_isNonblocking)
            {
                res = splice(pipeFd, nullptr, m_fd.get(), nullptr, _size, SPLICE_F_MORE | SPLICE_F_NONBLOCK);
            }
            else
            {
                res = splice(pipeFd, nullptr, m_fd.get(), nullptr, _size, SPLICE_F_MORE);
            }
        }
        default:
            THROW_SOCKET_EXCEPTION("Unhandled socket type: " << m_addr.socketType());
            break;
    }
    
    if(res == -1)
        THROW_SOCKET_EXCEPTION("Could not splice to Socket - " << strerror(errno));
    return res;
};

ssize_t BFSocket::canRead()
{
    int result = 0;
    if(ioctl(m_fd.get(), FIONREAD, &result) == -1)
        THROW_SOCKET_EXCEPTION("Could not realize can read - " << strerror(errno));
    return result;
};

ssize_t BFSocket::canWrite()
{
#ifdef FIONSPACE
    int result = 0;
    if(ioctl(m_fd.get(), FIONSPACE, &result) == -1)
        THROW_SOCKET_EXCEPTION("Could not realize can write - " << strerror(errno));
    return result;
#else
    int result = 0;
    if(ioctl(m_fd.get(), TIOCOUTQ, &result) == -1)
        THROW_SOCKET_EXCEPTION("Could not realize can read - " << strerror(errno));
    return m_sendBufferSize - result;
#endif
};

BFSocketPtr BFSocket::acceptClient()
{
    FileDescriptor fd;
    fd.set(accept(m_fd.get(), nullptr, 0));
    if(fd.get() == -1)
        THROW_SOCKET_EXCEPTION("Could not accept tcp client - " << strerror(errno));
    
    ServiceAddress sdr(serviceAddress().url(), ServiceAddress::stTCP);
    return BFSocketPtr(new BFSocket(sdr, std::move(fd)));
}

void BFSocket::setNonblocking()
{
    int flags = fcntl(m_fd.get(), F_GETFL,0);
    if(flags == -1)
        THROW_SOCKET_EXCEPTION("Could not make socket nonblocking - " << strerror(errno));
    if(fcntl(m_fd.get(), F_SETFL, flags | O_NONBLOCK) == -1)
        THROW_SOCKET_EXCEPTION("Could not make socket nonblocking - " << strerror(errno));
    m_isNonblocking = true;
}

void BFSocket::setMulticastTTL(int _TTL)
{
    auto size = sizeof(_TTL);
    setsockopt(m_fd.get(), IPPROTO_IP, IP_MULTICAST_TTL, &_TTL, size);
}

}
