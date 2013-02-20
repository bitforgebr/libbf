/*
* bf.h
*
*  Created on: Sep 03, 2012
*      Author: gianni
*
* BitForge http://www.bitforge.com.br
* Copyright (c) 2012 All Right Reserved,
*/

#ifndef __INCLUDE_LIBBF_BF_H_
#define __INCLUDE_LIBBF_BF_H_

#define UNUSED(ARG) do { (void(ARG)) } while (false)

#include <iostream>
#include <ctime>
#include <math.h>
#include <unistd.h>

#include <bf/ncstring.h>

namespace bitforge
{

template<typename T>
class StrongTypedef
{
private:
    T   m_value;
    
public:
    StrongTypedef(): m_value(T()) {}
    
    static StrongTypedef<T> make(const T& v)
    {
        StrongTypedef<T> result;
        result.m_value = v;
        return result;
    }
    
    static StrongTypedef<T> make(T &&v)
    {
        StrongTypedef<T> result;
        result.m_value = v;
        return result;
    }
    
    StrongTypedef(T) = delete;
    StrongTypedef<T>& operator=(const T&) = delete;
    
    T get() const { return m_value; }
    void set(T v)  { m_value = v; }
    
    bool operator==(const StrongTypedef<T>& other) const { return m_value == other.m_value; }
    bool operator!=(const StrongTypedef<T>& other) const { return m_value != other.m_value; }
    bool operator>(const StrongTypedef<T>& other) const { return m_value >  other.m_value; }
    bool operator<(const StrongTypedef<T>& other) const { return m_value <  other.m_value; }
};

template<typename T>
class SingletonStrongTypedef
{
private:
    T   m_value;
    
public:
    SingletonStrongTypedef(): m_value(T()) {}
    
    static SingletonStrongTypedef<T> make(T &&v)
    {
        SingletonStrongTypedef<T> result;
        result.m_value = v;
        return result;
    }
    
    SingletonStrongTypedef(T) = delete;
    SingletonStrongTypedef<T>& operator=(const T&) = delete;
    
    T get() const { return m_value; }
    void set(T v)  { m_value = v; }
    
    bool operator==(const SingletonStrongTypedef<T>& other) const { return m_value == other.m_value; }
    bool operator!=(const SingletonStrongTypedef<T>& other) const { return m_value != other.m_value; }
    bool operator>(const SingletonStrongTypedef<T>& other) const { return m_value >  other.m_value; }
    bool operator<(const SingletonStrongTypedef<T>& other) const { return m_value <  other.m_value; }
};
    
template <typename R, typename L>
class StringsComparer
{
private:
    static const char* get_char_str(NCString s) { return s.c_str(); }
    static const char* get_char_str(::std::string s) { return s.c_str(); }
    static const char* get_char_str(const char *s) { return s; }
public:
    static bool IsEqual(L str1, R str2)
    {
        return strcmp(get_char_str(str1), get_char_str(str2)) == 0;
    }

    static bool IsEqualICase(L str1, R str2)
    {
        return strcasecmp(get_char_str(str1), get_char_str(str2)) == 0;
    }
};

template<typename L, typename R>
bool CompStr(L str1, R str2)
{
    return StringsComparer<R, L>::IsEqual(str1, str2);
}

template<typename L, typename R>
bool ICompStr(L str1, R str2)
{
    return StringsComparer<R, L>::IsEqualICase(str1, str2);
}

class ExceptionWithMessage: public std::exception
{
public:
    ExceptionWithMessage(std::string what) throw() : m_what(what) {};
    virtual ~ExceptionWithMessage() throw() {};

private:
    NCString m_what;

public:
    virtual const char* what() const throw()
    {
        return m_what.data();
    };
};

class ErrnoException: public ExceptionWithMessage
{
private:
    int m_errno;

public:
    ErrnoException(int error_no);
    ErrnoException(std::string err_msg, int error_no);

    int error_number() const { return m_errno; }
};

enum DateFormat
{
    dfSQL,
    dfCompact
};

inline std::string getDate(DateFormat dateFormat = dfSQL)
{
    char dateStr[32];

    time_t t;
    struct tm tm;

    ::std::time(&t);
    gmtime_r(&t, &tm);

    const char* format;

    switch(dateFormat)
    {
        case dfSQL:
            format = "'%.4d-%.2d-%.2d %.2d:%.2d:%.2d'";
            break;

        case dfCompact:
        default:
            format = "%.4d%.2d%.2d%.2d%.2d%.2d";
            break;
    }

    ::std::snprintf(dateStr, sizeof(dateStr), format, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    return dateStr;
}

class ProcTimer
{
private:
    struct timespec m_time;

    ProcTimer(struct timespec ts) : m_time(ts) {}

public:
    ProcTimer()
    {
        clock_gettime(CLOCK_REALTIME, &m_time);
    }

    ProcTimer operator- (const ProcTimer& other) const
    {
        struct timespec ts = other.m_time;

        if(m_time.tv_nsec > ts.tv_nsec)
        {
            ts.tv_nsec = m_time.tv_nsec - ts.tv_nsec;
        }
        else
        {
            auto d = ts.tv_nsec - m_time.tv_nsec;
            ts.tv_sec--;
            ts.tv_nsec = pow10(9) - d;
        }

        ts.tv_sec = m_time.tv_sec - ts.tv_sec;

        return ProcTimer(ts);
    }

    double seconds() const
    {
        return static_cast<double>(m_time.tv_sec) + (static_cast<double>(m_time.tv_nsec) / pow10(9));
    }

    double operator/ (double value) const
    {
        return value / seconds();
    }

    bool operator> (int value) const
    {
        return seconds() > value;
    }
};

inline std::ostream& operator<< (std::ostream& out, const ProcTimer& timer)
{
    out << round(timer.seconds() * 100) / 100;
    return out;
}

template<typename T>
void zero_init(T& t)
{
    memset(&t, 0, sizeof(T));
}

template<typename T>
bool is_zero(const T& t)
{
    const char* p = reinterpret_cast<const char*>(&t);
    const char* end = p + (sizeof(T) % sizeof(int));

    while(p != end)
    {
        if(*p) return false;

        p++;
    }

    const int* i = reinterpret_cast<const int*>(p);

    const int* iend = i + (sizeof(T) - (sizeof(T) % sizeof(int)));

    while(i != iend)
    {
        if(*i) return false;

        i++;
    }

    return true;
}

// TODO - Check for a better solution
// This is used instead of snprintf or lexical_cast due to their lack of availability
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

inline char* inttostr(char* buffer, const int bufferSize, int val)
{
    const bool neg = val < 0;
    char* result =  const_cast<char*>(uinttostr(buffer, bufferSize, val));

    if(neg)
        *--result = '-';

    return result;
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


inline char* uinttohex(char* buffer, const int bufferSize, unsigned int val)
{
    return uinttostr(buffer, bufferSize, val, 16);
}

inline char* inttohex(char* buffer, const int bufferSize, int val)
{
    return uinttostr(buffer, bufferSize, val, 16);
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

inline char* int64tostr(char* buffer, const int bufferSize, int64_t val)
{
    const bool neg = val < 0;
    char* result =  const_cast<char*>(uint64tostr(buffer, bufferSize, val));

    if(neg)
        *--result = '-';

    return result;
}

//TODO Fixme
inline unsigned char hextoint(const char* v)
{
    if (strlen(v) < 2)
        return 0;

    unsigned char res;

    if (v[0] >= '0' && v[0] <= '9')
        res = (v[0] - '0') << 4;
    else if (v[0] >= 'a' && v[0] <= 'f')
        res = ((v[0] - 'f') + 15) << 4;
    else if (v[0] >= 'A' && v[0] <= 'F')
        res = ((v[0] - 'F') + 15) << 4;

    if (v[1] >= '0' && v[1] <= '9')
        res += v[1] - '0';
    else if (v[1] >= 'a' && v[1] <= 'f')
        res += (v[1] - 'f') + 15;
    else if (v[1] >= 'A' && v[1] <= 'F')
        res += (v[1] - 'F') + 15;

    return res;
}

//TODO Fixme
inline std::string intohex(unsigned char v)
{
    char result[] = "00";
    char* c = &result[2];

    do
    {
        const unsigned char digit = (v % 16);

        if(digit <= 9)
            *--c = '0' + digit;
        else
            *--c = 'A' + digit - 10;

        v /= 16;
    }
    while(v);

    return result;
}

constexpr unsigned int strKey(const char* str)
{
    return ((!str) || (!str[0])) ? 0 :
        (!str[1]) ? (str[0] << 24) :
        (!str[2]) ? (str[0] << 24) + (str[1] << 16) :
        (!str[3]) ? (str[0] << 24) + (str[1] << 16) + (str[2] << 8) :
        (str[0] << 24) + (str[1] << 16) + (str[2] << 8) + (str[3]);
}

inline unsigned int strKey(const std::string& str)
{
    return strKey(str.c_str());
}

constexpr unsigned int striKey(const char* str)
{
    return ((!str) || (!str[0])) ? 0 :
        (!str[1]) ? ((str[0] | 0x20) << 24) :
        (!str[2]) ? ((str[0] | 0x20) << 24) + ((str[1] | 0x20) << 16) :
        (!str[3]) ? ((str[0] | 0x20) << 24) + ((str[1] | 0x20) << 16) + ((str[2] | 0x20) << 8) :
        ((str[0] | 0x20) << 24) + ((str[1] | 0x20) << 16) + ((str[2] | 0x20) << 8) + (str[3] | 0x20);
}

inline unsigned int striKey(const std::string& str)
{
    return striKey(str.c_str());
}

// Get the number of cores reported by the OS.
int getNumCores();


struct ProcStreams
{
    int stdIn;
    int stdOut;
    int stdErr;

    ProcStreams(): stdIn(0), stdOut(1), stdErr(3) {}

    ~ProcStreams()
    {
        close(stdIn);
        close(stdOut);
        close(stdErr);
    }
};

/**
*	Start a process, but connect the process' stdin, stdout and strerr to pipes which are then
*  returned to the caller in the @streams param.
*  @param streams out structre to recieve the pipes for the process's streams.
*  @param args process name and arguments.  args[0] should be the exec name (i.e. /bin/ls)
*  @param env enviroment for the new process.  The default is a copy of the current process' enviroment.
*/
bool runAttachedProcess(ProcStreams* streams, const char* const args[], const char* const env[] = environ);

/**
 * Get the OS's memory page size
 */
std::size_t getSystemPageSize();

/* Fast hash function */
uint32_t fletcher32(uint16_t* data, ::std::size_t len);

}

#endif // __INCLUDE_LIBBF_BF_H_
