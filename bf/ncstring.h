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

template<typename T>
class BasicNCString
{
private:
    typedef std::shared_ptr<T> CharRef;

    CharRef         m_memory;
    char*           m_begin = nullptr;
    std::size_t     m_length = 0;
    std::size_t     m_maxSize = 0;

    void alloc(std::size_t size)
    {
        m_maxSize = size;
        m_memory = std::make_shared<T>(size);
        m_begin = m_memory->get();
    }

public:
    static const std::size_t npos = std::string::npos;

    BasicNCString() {}

    BasicNCString(const BasicNCString& other)
    {
        m_length = other.m_length;
        m_memory = other.m_memory;
        m_begin = m_memory ? m_memory->get() : nullptr;
    }

    BasicNCString& operator=(const BasicNCString& other)
    {
        m_length = other.m_length;
        m_memory = other.m_memory;
        m_begin = m_memory ? m_memory->get() : nullptr;

        return *this;
    }

    BasicNCString(BasicNCString&& other)
    {
        std::swap(m_memory, other.m_memory);
        std::swap(m_begin, other.m_begin);
        std::swap(m_length, other.m_length);
    }

    BasicNCString& operator=(BasicNCString&& other)
    {
        std::swap(m_memory, other.m_memory);
        std::swap(m_begin, other.m_begin);
        std::swap(m_length, other.m_length);
        return *this;
    }

    BasicNCString(const std::string &str)
    {
        m_length = str.length();
        alloc(m_length + 1);
        memcpy(m_begin, str.c_str(), m_length);
        m_begin[m_length] = 0;
    }

    BasicNCString& operator=(const std::string &str)
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

    BasicNCString(const char* str)
    {
        m_length = strlen(str);
        alloc(m_length + 1);
        memcpy(m_begin, str, m_length);
        m_begin[m_length] = 0;
    }

    BasicNCString& operator=(const char* str)
    {
        assign(str);
        return *this;
    }

    void assign(const char* str)
    {
        assign(str, strlen(str));
    }

    void assign(const char* str, std::size_t len)
    {
        m_length = len;
        alloc(m_length + 1);
        memcpy(m_begin, str, m_length);
        m_begin[m_length] = 0;
    }

    BasicNCString(const char* str, std::size_t length) :
        m_length(length)
    {
        alloc(m_length + 1);
        memcpy(m_begin, str, m_length);
        m_begin[m_length] = 0;
    }

    bool operator<(const BasicNCString& other) const
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

    bool operator==(const BasicNCString& other) const
    {
        return std::strncmp(begin(), other.begin(), m_length) == 0;
    }

    bool operator!=(const BasicNCString& other) const
    {
        return std::strncmp(begin(), other.begin(), m_length) != 0;
    }
    
    char& operator[](std::size_t pos)
    {
        if (pos < m_length)
            return m_begin[pos];
        else
            return m_begin[m_length];
    }
    
    const char& operator[](std::size_t pos) const
    {
        if (pos < m_length)
            return m_begin[pos];
        else
            return m_begin[m_length];
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
            const char *oldBegin = m_begin;
            alloc(newLen + 1);

            memcpy(m_begin, oldBegin, m_length);
            memcpy(m_begin + m_length, str, strLen);
        }

        m_length = newLen;
        m_begin[m_length] = 0;
    }

    void append(const std::string& str)
    {
        append(str.c_str());
    }

    BasicNCString& operator+=(const char* str)
    {
        append(str);
        return *this;
    }

    BasicNCString& operator+(const char* str)
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

    int compare(const BasicNCString& other) const
    {
        return std::strcmp(m_begin, other.m_begin);
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

    BasicNCString substr(std::size_t start) const
    {
        std::size_t l = m_length - start;
        return BasicNCString(m_begin + start, l);
    }

    BasicNCString substr(std::size_t start, std::size_t length) const
    {
        std::size_t l = std::min(length, m_length - start);
        return BasicNCString(m_begin + start, l);
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
                const char* oldBegin = m_begin;

                alloc(length + 1);
                m_length = length;

                memcpy(m_begin, oldBegin, length);

                m_begin[m_length] = 0;
            }
        }
    }
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const BasicNCString<T>& string)
{
    if (string.length())
        stream.write(string.data(), string.length());

    return stream;
}

class BasicMemoryObject
{
private:
    char* m_memory;

public:
    BasicMemoryObject(const std::size_t size)
    {
        m_memory = new char[size];
    }

    ~BasicMemoryObject()
    {
        delete[] m_memory;
    }

    char* get() { return m_memory; }
};

typedef BasicNCString<BasicMemoryObject> NCString;

} // bitforge

#endif // __INCLUDE_LIBBF_RAWSTRING_H_

