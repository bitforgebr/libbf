
#ifndef _INCLUDE_CIRCULARBUFFER_H
#define _INCLUDE_CIRCULARBUFFER_H

#include <cassert>
#include <cstdint>
#include <cstring>

#include <algorithm>

namespace bitforge {

template <typename T>
class CircularBuffer
{
public:
    typedef std::size_t size_t;

    CircularBuffer( size_t size ):
        m_bufferSize( size ),
        m_availRead( 0 ),
        m_availWrite( size )
    {
        m_buffer = new T[ size ];
        m_bufferEnd = m_buffer + size;

        m_posRead = m_buffer;
        m_posWrite = m_buffer;
    }

    ~CircularBuffer()
    {
        delete[] m_buffer;
    }

protected:
    const size_t m_bufferSize;
    T*  m_buffer;
    T*  m_bufferEnd;

    T*  m_posRead;
    size_t m_availRead;

    T* m_posWrite;
    size_t m_availWrite;

    void doWrite(const T* x, size_t size)
    {
        assert(m_posWrite + size <= m_bufferEnd);

        memcpy(m_posWrite, x, size);
        m_posWrite += size;

        if (m_posWrite == m_bufferEnd)
            m_posWrite = m_buffer;
    }

    void doRead(T* x, size_t size, T*& readVar)
    {
        assert(readVar + size <= m_bufferEnd);

        memcpy(x, readVar, size);
        readVar += size;

        if (readVar == m_bufferEnd)
            readVar = m_buffer;
    }
public:
    size_t availableReadSize() const
    {
        return m_availRead;
    }

    size_t availableWriteSize() const
    {
        return m_availWrite;
    }

    size_t push (const T* x, size_t size = 1)
    {
        if ( size == 0 )
            return 0;
        if ( size > m_availWrite )
            size = m_availWrite;

        m_availRead += size;
        m_availWrite -= size;

        size_t remain = size;

        while(remain)
        {
            size_t sz = std::min(static_cast<size_t>(m_bufferEnd - m_posWrite), remain);
            doWrite(x, sz);
            remain -= sz;
            x += sz;
        }

        return size;
    }

    size_t pop (T* x, size_t size = 1)
    {
        if ( size == 0 )
            return 0;
        if ( size > m_availRead )
            size = m_availRead;

        m_availRead -= size;
        m_availWrite += size;

        size_t remain = size;

        while(remain)
        {
            size_t sz = std::min(static_cast<size_t>(m_bufferEnd - m_posRead), remain);
            doRead(x, sz, m_posRead);
            remain -= sz;
            x += sz;
        }

        return size;
    }

    size_t peek(T* x, size_t size = 1)
    {
        if ( size == 0 )
            return 0;
        if ( size > m_availRead )
            size = m_availRead;

        size_t remain = size;
        T* pos = m_posRead;

        while(remain)
        {
            size_t sz = std::min(static_cast<size_t>(m_bufferEnd - pos), remain);
            doRead(x, sz, pos);
            remain -= sz;
            x += sz;
        }

        return size;
    }
};

}

#endif // _INCLUDE_CIRCULARBUFFER_H
