/*
 * rawstring.h
 *
 *  Created on: Apr 05, 2012
 *      Author: gianni
 *
 * BitForge http://www.bitforge.com.br
 * Copyright (c) 2012 All Right Reserved,
 */

#ifndef __INCLUDE_LIBBF_RAWSTRING_H_
#define __INCLUDE_LIBBF_RAWSTRING_H_

#include <string>
#include <cstring>
#include <memory>
#include <ostream>

namespace bitforge
{

/**
 * @class NSString - No-copy string
 * @description Thread-safe string that breaks std::string COW's problems
 */

class NCString
{
private:
    class MemoryObject
    {
    private:
        char* m_memory;

    public:
        MemoryObject(const std::size_t size)
        {
            m_memory = new char[size];
        }

        ~MemoryObject()
        {
            delete[] m_memory;
        }

        char* get() { return m_memory; }
    };

    typedef std::shared_ptr<MemoryObject> CharRef;

    CharRef         m_memory;
    char*           m_begin = nullptr;
    std::size_t     m_length = 0;
    std::size_t     m_maxSize = 0;

    void alloc(std::size_t size)
    {
        m_maxSize = size;
        m_memory = std::make_shared<MemoryObject>(size);
        m_begin = m_memory->get();
    }

public:
    static const std::size_t npos = std::string::npos;

    NCString() {}

    NCString(const NCString& other)
    {
        m_length = other.m_length;
        m_memory = other.m_memory;
        m_begin = m_memory ? m_memory->get() : nullptr;
    }

    NCString& operator=(const NCString& other)
    {
        m_length = other.m_length;
        m_memory = other.m_memory;
        m_begin = m_memory ? m_memory->get() : nullptr;

        return *this;
    }

    NCString(NCString&& other)
    {
        std::swap(m_memory, other.m_memory);
        std::swap(m_begin, other.m_begin);
        std::swap(m_length, other.m_length);
    }

    NCString& operator=(NCString&& other)
    {
        std::swap(m_memory, other.m_memory);
        std::swap(m_begin, other.m_begin);
        std::swap(m_length, other.m_length);
        return *this;
    }

    NCString(const std::string &str)
    {
        m_length = str.length();
        alloc(m_length + 1);
        memcpy(m_begin, str.c_str(), m_length);
        m_begin[m_length] = 0;
    }

    NCString& operator=(const std::string &str)
    {
        m_length = str.length();
        alloc(m_length + 1);
        memcpy(m_begin, str.c_str(), m_length);
        m_begin[m_length] = 0;

        return *this;
    }

    operator std::string() const
    {
        if (m_begin && m_length)
            return std::string(m_begin, m_length);
        return std::string();
    }

    NCString(const char* str)
    {
        m_length = strlen(str);
        alloc(m_length + 1);
        memcpy(m_begin, str, m_length);
        m_begin[m_length] = 0;
    }

    NCString& operator=(const char* str)
    {
        m_length = strlen(str);
        alloc(m_length + 1);
        memcpy(m_begin, str, m_length);
        m_begin[m_length] = 0;

        return *this;
    }

    NCString(const char* str, std::size_t length) :
        m_length(length)
    {
        alloc(m_length + 1);
        memcpy(m_begin, str, m_length);
        m_begin[m_length] = 0;
    }

    bool operator<(const NCString& other) const
    {
        const char* t = data();
        const char* o = other.data();
        if (t == nullptr)
            return true;
        else if (o == nullptr)
            return false;
        else
            return std::strcmp(t, o) < 0;
    }

    bool operator==(const char* str) const
    {
        return std::strncmp(begin(), str, m_length) == 0;
    }

    bool operator==(const NCString other) const
    {
        return std::strncmp(begin(), other.begin(), m_length) == 0;
    }

    bool operator!=(const char* str) const
    {
        return std::strncmp(m_begin, str, m_length) != 0;
    }

    void append(const char* str)
    {
        const auto strLen = strlen(str);
        const auto newLen = m_length + strLen;

        if (m_memory.use_count() == 1 && newLen < m_maxSize)
        {
            char *c = m_begin + m_length;
            memcpy(c, str, strLen);
        }
        else
        {
            CharRef oldStr = m_memory;
            alloc(newLen + 1);

            memcpy(m_begin, oldStr.get(), m_length);
            memcpy(m_begin + m_length, str, strLen);
        }

        m_length = newLen;
        m_begin[m_length] = 0;
    }

    NCString& operator+=(const char* str)
    {
        append(str);
        return *this;
    }

    const char* data() const
    {
        return m_begin;
    }

    const char* c_str() const
    {
        return data();
    }

    const char* begin() const
    {
        return data();
    }

    const char* end() const
    {
        return data() + m_length;
    }

    std::size_t length() const
    {
        return m_length;
    }

    void clear()
    {
        m_memory.reset();
        m_begin = nullptr;
        m_length = 0;
    }

    bool empty() const
    {
        return m_length == 0;
    }

    int compare(const char* other) const
    {
        return std::strcmp(m_begin, other);
    }

    std::size_t find(char v) const
    {
        std::size_t res = 0;
        const char* c = m_begin;
        const char* e = m_begin + m_length;

        for(; c != e; c++, res++)
            if (*c == v)
                return res;

        return std::string::npos;
    }

    NCString substr(std::size_t start) const
    {
        std::size_t l = m_length - start;
        return NCString(m_begin + start, l);
    }

    NCString substr(std::size_t start, std::size_t length) const
    {
        std::size_t l = std::min(length, m_length - start);
        return NCString(m_begin + start, l);
    }

    void chop(std::size_t length)
    {
        if (m_length && length < m_length)
        {
            if (m_memory.use_count() == 1)
            {
                const_cast<char*>(m_begin)[length] = 0;
                m_length = length;
            }
            else
            {
                CharRef oldRef = m_memory;

                alloc(length + 1);
                m_length = length;

                memcpy(m_begin, oldRef.get(), length);

                m_begin[m_length] = 0;
            }
        }
    }
};

inline std::ostream& operator<<(std::ostream& stream, const NCString& string)
{
    if (string.length())
        stream.write(string.data(), string.length());

    return stream;
}

} // bitforge

#endif // __INCLUDE_LIBBF_RAWSTRING_H_
