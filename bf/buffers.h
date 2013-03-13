/*
 * bf.h
 *
 *  Created on: Feb 17, 2013
 *      Author: gianni
 *
 * BitForge http://www.bitforge.com.br
 * Copyright (c) 2012 All Right Reserved,
 */

#ifndef __INCLUDE_LIBBF_BUFFERS_H_
#define __INCLUDE_LIBBF_BUFFERS_H_

#include <memory>
#include <stack>
#include <vector>

#include <bf/bf.h>

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

class MemoryPool
{
public:
    typedef std::size_t size_type;
    
    MemoryPool(size_type minNumberPageCahce = 1, size_type pageSize = getSystemPageSize()): 
    m_minNumberPageCahce(minNumberPageCahce), m_pageSize(pageSize) {};
    
    class MemoryPage
    {
    private:
        MemoryPool  *m_parent;
        void        *m_data;
        
    protected:
        friend class MemoryPool;
        
        MemoryPage(MemoryPool *parent, void *data): m_parent(parent), m_data(data) {}
        
    public:
        MemoryPage(const MemoryPage&) = delete;
        void operator=(const MemoryPage&) = delete;
        
        MemoryPage(MemoryPage&& other): m_parent(other.m_parent), m_data(other.m_data) { other.m_data = nullptr; }
        MemoryPage& operator=(MemoryPage&& other) 
        { 
            m_parent = other.m_parent; 
            m_data = other.m_data; 
            other.m_data = nullptr; 
            return *this; 
        }
        
        ~MemoryPage() 
        { 
            if (m_data)
            {
                m_parent->returnPage( this ); 
                m_data = nullptr;
            }
        }
        
        void* data() { return m_data; }
    };
    
private:
    size_type   m_minNumberPageCahce;
    size_type   m_pageSize;
    
    std::stack<void*> m_pageStack;
    
protected:
    friend class MemoryPage;
    inline void returnPage(MemoryPage *page);
    
public:
    typedef std::unique_ptr<MemoryPool::MemoryPage> MemoryPagePtr;
    inline MemoryPagePtr getPage();
    size_type  pageSize() const { return m_pageSize; }
};
typedef std::shared_ptr<MemoryPool> MemoryPoolPtr;

void MemoryPool::returnPage(MemoryPool::MemoryPage* page)
{
    if (m_pageStack.size() < m_minNumberPageCahce)
        m_pageStack.push(page->data());
    else
    {
        char* ptr = static_cast<char*>(page->data());
        delete[] ptr;
    }
}

MemoryPool::MemoryPagePtr MemoryPool::getPage()
{
    void *ptr;
    if (!m_pageStack.empty())
    {
        ptr = m_pageStack.top();
        m_pageStack.pop();
    }
    else
        ptr = new char[m_pageSize];
    
    return MemoryPagePtr(new MemoryPage(this, ptr));
}


template<typename T>
class SimpleBuffer
{
public:
    typedef std::size_t size_type;
    typedef std::vector<std::pair<MemoryPool::MemoryPagePtr, size_type>> MemoryVector;
    
    SimpleBuffer(MemoryPoolPtr __pool = std::make_shared<MemoryPool>()) : m_pool(__pool) {}
    SimpleBuffer(T *v, size_type n, MemoryPoolPtr __pool = std::make_shared<MemoryPool>()) : m_pool(__pool) { write(v, n); }
    
    class Iterator
    {
    protected:
        SimpleBuffer<T> *m_parent;
        MemoryVector::iterator m_it;
        
    public:
        Iterator(SimpleBuffer<T> *parent, MemoryVector::iterator it): 
        m_parent(parent), m_it(it)
        {
            if (m_it != m_parent->m_data.end())
            {
                data = static_cast<T*>(it->first->data());
                size = it->second;
            }
        }
        
        T* data = nullptr;
        size_type size = 0;
        
        Iterator& operator++(int)  
        { 
            ++m_it;
            
            if (m_it != m_parent->m_data.end())   
            {
                data = static_cast<T*>(m_it->first->data());
                size = m_it->second;
            }
            else
            {
                data = nullptr;
                size = 0;
            }
            return *this;
        }
        
        bool operator==(const Iterator &other) const { return m_it == other.m_it; }
        bool operator!=(const Iterator &other) const { return m_it != other.m_it; }
    };
private:
    MemoryPoolPtr m_pool;
    
    MemoryVector m_data;
    
    T* m_writePos = nullptr;
    
    size_type m_size = 0;
    size_type m_availWrite = 0;
    
    void getNewPage()
    {
        MemoryPool* pool = m_pool.get();
        MemoryPool::MemoryPagePtr page = pool->getPage();
        m_writePos = static_cast<T*>(page->data());
        m_data.push_back(std::make_pair(std::move(page), 0));
        m_availWrite += pool->pageSize();
    }
    
public:
    size_type size() const { return m_size; }
    size_type pageSize() const { return m_pool->pageSize(); }
    
    void append(T *v, size_type n)
    {
        while(n)
        {
            if(m_availWrite == 0)
                getNewPage();
            
            const size_type sz = std::min(m_availWrite, n);
            memcpy(m_writePos, v, sz);
            
            m_writePos += sz;
            v += sz;
            n -= sz;
            m_size += sz;
            m_availWrite -= sz;
            m_data.rbegin()->second += sz;
        }
    }
    
    size_type peek(T *v, size_type n)
    {
        size_type result = 0;
        
        auto it = m_data.begin();
        while(n && it != m_data.end())
        {
            char *ptr = static_cast<T*>(it->first->data());
            const size_type sz = std::min(it->second, n);
            
            memcpy(v, ptr, sz);
            
            v += sz;
            n -= sz;
            result += sz;
            ++it;
        }
        
        return result;
    }
    
    Iterator begin() { return Iterator(this, m_data.begin()); };
    Iterator end() { return Iterator(this, m_data.end()); };
};

}

#endif // __INCLUDE_LIBBF_BUFFERS_H_
