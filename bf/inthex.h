/*
 * bf.h
 *
 *  Created on: 2013-03-12
 *      Author: gianni
 *
 * BitForge http://www.bitforge.com.br
 * Copyright (c) 2012 All Right Reserved,
 */

#ifndef __INCLUDE_LIBBF_INTHEX_H_
#define __INCLUDE_LIBBF_INTHEX_H_

#define UNUSED(ARG) do { (void(ARG)) } while (false)

#include <iostream>
#include <ctime>
#include <math.h>
#include <unistd.h>

#include <bf/ncstring.h>

namespace bitforge
{

// These are used instead of snprintf, lexical_cast or stringstream due to their lack of availability

inline char* uinttostr(char* buffer, const int bufferSize, unsigned int val)
{
    char* c = buffer + (bufferSize - 1);
    *c = 0;

    do
    {
        *--c = '0' + (val % 10);
        val /= 10;
    }
    while(val);

    return c;
}

inline std::string uinttostr(unsigned int val)
{
    char buffer[32];
    return uinttostr(buffer, sizeof(buffer), val);
}

inline char* inttostr(char* buffer, const int bufferSize, int val)
{
    const bool neg = val < 0;
    char* result =  const_cast<char*>(uinttostr(buffer, bufferSize, abs(val)));

    if(neg)
        *--result = '-';

    return result;
}

inline std::string inttostr(unsigned int val)
{
    char buffer[32];
    return inttostr(buffer, sizeof(buffer), val);
}

inline char* uinttostr(char* buffer, const int bufferSize, unsigned int val, int base)
{
    char* c = buffer + (bufferSize - 1);
    *c = 0;
    
    do
    {
        const unsigned int digit = (val % base);
        
        if(digit <= 9)
            *--c = '0' + digit;
        else
            *--c = 'A' + digit - 10;
        
        val /= base;
    }
    while(val);
    
    return c;
}

inline std::string uinttostr(unsigned int val, int base)
{
    char buffer[32];
    return uinttostr(buffer, sizeof(buffer), val, base);
}


inline char* uinttohex(char* buffer, const int bufferSize, unsigned int val)
{
    return uinttostr(buffer, bufferSize, val, 16);
}

inline std::string uinttohex(unsigned int val)
{
    char buffer[32];
    return uinttohex(buffer, sizeof(buffer), val);
}

inline char* inttohex(char* buffer, const int bufferSize, int val)
{
    return uinttostr(buffer, bufferSize, abs(val), 16);
}

inline std::string inttohex(unsigned int val)
{
    char buffer[32];
    return inttohex(buffer, sizeof(buffer), val);
}

inline char* uint64tostr(char* buffer, const int bufferSize, uint64_t val)
{
    char* c = buffer + (bufferSize - 1);
    *c = 0;
    
    do
    {
        *--c = '0' + (val % 10);
        val /= 10;
    }
    while(val);
    
    return c;
}

inline std::string uint64tostr(uint64_t val)
{
    char buffer[64];
    return uint64tostr(buffer, sizeof(buffer), val);
}

inline char* int64tostr(char* buffer, const int bufferSize, int64_t val)
{
    const bool neg = val < 0;
    char* result =  const_cast<char*>(uint64tostr(buffer, bufferSize, llabs(val)));
    
    if(neg)
        *--result = '-';
    
    return result;
}

inline std::string int64tostr(uint64_t val)
{
    char buffer[64];
    return int64tostr(buffer, sizeof(buffer), val);
}

inline uint hextoint(const char* v)
{
    uint res = 0;
    
    while(*v)
    {
        res <<= 4;
        
        if (v[0] >= '0' && v[0] <= '9')
            res += (v[0] - '0');
        else if (v[0] >= 'a' && v[0] <= 'f')
            res += ((v[0] - 'f') + 15);
        else if (v[0] >= 'A' && v[0] <= 'F')
            res += ((v[0] - 'F') + 15);
        
        v++;
    }
    
    return res;
}

inline uint hextoint(const std::string& v)
{
    return hextoint(v.c_str());
}

inline uint64_t hextoint64(const char* v)
{
    uint64_t res = 0;
    
    while(*v)
    {
        res <<= 4;
        
        if (v[0] >= '0' && v[0] <= '9')
            res += (v[0] - '0');
        else if (v[0] >= 'a' && v[0] <= 'f')
            res += ((v[0] - 'f') + 15);
        else if (v[0] >= 'A' && v[0] <= 'F')
            res += ((v[0] - 'F') + 15);
        
        v++;
    }
    
    return res;
}

inline uint64_t hextoint64(const std::string& v)
{
    return hextoint64(v.c_str());
}

}

#endif // __INCLUDE_LIBBF_INTHEX_H_
