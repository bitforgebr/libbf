#ifndef __INCLUDE_BFIO_H
#define __INCLUDE_BFIO_H

#include <bf/bf.h>

#include <unistd.h>
#include <sys/select.h>
#include <sstream>

namespace bitforge {

#ifndef NDEBUG
#define THROW_BFIO_EXCEPTION(STR)                             \
    do { ::std::stringstream ss;                                \
        ss << basename(__FILE__) << ':' << __LINE__ << ':' << __PRETTY_FUNCTION__ <<  \
        STR; throw IOException(ss.str(), errno);            \
    } while (false)

#else
#define THROW_BFIO_EXCEPTION(STR) do { ::std::stringstream ss; ss << STR; throw IOException(ss.str(), errno); } while (false)
#endif

typedef ::std::size_t   size_t;
typedef ssize_t         ssize_t;

typedef SingletonStrongTypedef<int> FileDescriptor;

class IOException: public ErrnoException
{
public:
    IOException(int error_no): ErrnoException(error_no) {};
    IOException(std::string err_msg, int error_no): ErrnoException(err_msg, error_no) {};
};

template<typename T>
class IntrusiveListItem
{
public:
    typedef std::shared_ptr<T> shared_ptr_type;

    shared_ptr_type next() {
        return m_next;
    }

    shared_ptr_type prev() {
        return m_prev;
    }

protected:
    shared_ptr_type m_next;
    shared_ptr_type m_prev;

    void insertNext(shared_ptr_type _sharedThis, shared_ptr_type _next)
    {
        _next->m_prev = _sharedThis;
        _next->m_next = m_next;
        m_next = _next;
    }

    void insertPrev(shared_ptr_type _sharedThis, shared_ptr_type _prev)
    {
        _prev->m_prev = m_prev;
        _prev->m_next = _sharedThis;
        m_prev = _prev;
    }
};

class BFIO: public IntrusiveListItem<BFIO>, public std::enable_shared_from_this<BFIO>
{
public:
    virtual ssize_t read(void *, size_t) = 0;
    virtual ssize_t write(const void *, size_t) = 0;
    virtual ssize_t canRead() = 0;
    virtual ssize_t canWrite() = 0;

    BFIO& operator<<(std::string str)
    {
        write(str.data(), str.length());
        return *this;
    }
};

class BFSimpleFd: public BFIO
{
protected:
    FileDescriptor  m_fd;

public:
    BFSimpleFd();
    BFSimpleFd(FileDescriptor fd);

    virtual ~BFSimpleFd();

    virtual ssize_t read(void *buffer, size_t bufferSize);
    virtual ssize_t write(const void *buffer, size_t bufferSize);

    virtual ssize_t canRead();
    virtual ssize_t canWrite();
};

}

#endif // __INCLUDE_BFIO_H
