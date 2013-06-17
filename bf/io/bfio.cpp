#include "bfio.h"

namespace bitforge {

BFSimpleFd::BFSimpleFd()
{
};

BFSimpleFd:: BFSimpleFd(FileDescriptor fd): m_fd(fd)
{
}

BFSimpleFd::~BFSimpleFd()
{
    if(m_fd.get())
    {
        close(m_fd.get());
        m_fd = FileDescriptor(); // Reset to make sure it is == 0
    }
}

ssize_t BFSimpleFd::read(void *buffer, size_t bufferSize)
{
    ssize_t result = ::read(m_fd.get(), buffer, bufferSize);
    if (result == -1)
        THROW_BFIO_EXCEPTION("Error reading fd: '" << strerror(errno) << '\'');
    return result;
}

ssize_t BFSimpleFd::write(const void *buffer, size_t bufferSize)
{
    ssize_t result = ::write(m_fd.get(), buffer, bufferSize);
    if (result == -1)
        THROW_BFIO_EXCEPTION("Error writing to fd: '" << strerror(errno) << '\'');
    return result;
}

ssize_t BFSimpleFd::canRead()
{
    fd_set fdread, fdwrite, fdexcept;
    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcept);

    FD_SET(m_fd.get(), &fdread);

    int maxFd = m_fd.get() + 1;

    struct timeval timeout = { 0, 0 };
    int ret = select(maxFd, &fdread, &fdwrite, &fdexcept, &timeout);

    if (ret > 0)
    {
        return FD_ISSET(m_fd.get(), &fdread);
    }

    return 0;
};

ssize_t BFSimpleFd::canWrite()
{
    fd_set fdread, fdwrite, fdexcept;
    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcept);

    FD_SET(m_fd.get(), &fdread);

    int maxFd = m_fd.get() + 1;

    struct timeval timeout = { 0, 0 };
    int ret = select(maxFd, &fdread, &fdwrite, &fdexcept, &timeout);

    if (ret > 0)
    {
        return FD_ISSET(m_fd.get(), &fdwrite);
    }

    return 0;
};

};
