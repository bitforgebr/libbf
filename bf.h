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

#include "rawstring.h"

namespace bitforge
{

extern const char* PROGRAM_NAME;
extern bool terminateApp;

class ExceptionWithMessage: public std::exception
{
public:
	ExceptionWithMessage(std::string what) throw() : m_what(what) {};
	virtual ~ExceptionWithMessage() throw() {};

private:
	RawString m_what;

public:
	virtual const char* what() const throw() { return m_what.data(); };
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
	struct timespec m_time = {0,0};

	ProcTimer(struct timespec ts) : m_time(ts) {}

public:
	ProcTimer()
	{
		clock_gettime(CLOCK_REALTIME, &m_time);
	}

	ProcTimer operator-(const ProcTimer& other) const
	{
		struct timespec ts = other.m_time;

		if (m_time.tv_nsec > ts.tv_nsec)
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

	double operator/(double value) const
	{
		return value / seconds();
	}

	bool operator>(int value) const
	{
		return seconds() > value;
	}
};

inline std::ostream& operator<<(std::ostream& out, const ProcTimer& timer)
{
	out << round(timer.seconds() * 100) / 100;
	return out;
}

template<typename T>
void zero_init(T &t)
{
	memset(&t, 0, sizeof(T));
}

template<typename T>
bool is_zero(const T& t)
{
	const char *p = reinterpret_cast<const char*>(&t);
	const char *end = p + (sizeof(T) % sizeof(int));

	while(p != end)
	{
		if (*p) return false;
		p++;
	}

	const int *i = reinterpret_cast<const int*>(p);
	const int *iend = i + (sizeof(T) - (sizeof(T) % sizeof(int)));

	while(i != iend)
	{
		if (*i) return false;
		i++;
	}

	return true;
}

// TODO - Check for a better solution
// This is used instead of snprintf or lexical_cast due to their lack of availability
inline char* uinttostr(char *buffer, const int bufferSize, unsigned int val)
{
	char *c = buffer + (bufferSize - 1);
	*c = 0;

	do
	{
		*--c = '0' + ( val % 10 );
		val /= 10;
	} while( val );

	return c;
}

inline char* inttostr(char *buffer, const int bufferSize, int val)
{
	const bool neg = val < 0;
	char* result =  const_cast<char*>( uinttostr( buffer, bufferSize, val ) );
	if ( neg )
		*--result = '-';
	return result;
}

inline char* uinttostr(char *buffer, const int bufferSize, unsigned int val, int base)
{
	char *c = buffer + (bufferSize - 1);
	*c = 0;

	do
	{
		const unsigned int digit = ( val % base );
		if ( digit <= 9 )
			*--c = '0' + digit;
		else
			*--c = 'A' + digit - 10;

		val /= base;
	} while( val );

	return c;
}


inline char* uinttohex(char *buffer, const int bufferSize, unsigned int val)
{
	return uinttostr( buffer, bufferSize, val, 16 );
}

inline char* inttohex(char *buffer, const int bufferSize, int val)
{
	return uinttostr( buffer, bufferSize, val, 16 );
}

inline char* uint64tostr(char *buffer, const int bufferSize, uint64_t val)
{
	char *c = buffer + (bufferSize - 1);
	*c = 0;

	do
	{
		*--c = '0' + ( val % 10 );
		val /= 10;
	} while( val );

	return c;
}

inline char* int64tostr(char *buffer, const int bufferSize, int64_t val)
{
	const bool neg = val < 0;
	char* result =  const_cast<char*>( uint64tostr( buffer, bufferSize, val ) );
	if ( neg )
		*--result = '-';
	return result;
}

constexpr unsigned int strKey(const char* str)
{
	return
	((!str) || (!str[0])) ? 0 :
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
	return
	((!str) || (!str[0])) ? 0 :
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

}

#endif // __INCLUDE_LIBBF_BF_H_
