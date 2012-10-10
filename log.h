/*
 * log.h
 *
 *  Created on: Mar 1, 2012
 *      Author: gianni
 *
 * BitForge http://www.bitforge.com.br
 * Copyright (c) 2012 All Right Reserved,
 */

#ifndef __INCLUDE_LIBBF_LOG_H_
#define __INCLUDE_LIBBF_LOG_H_

#include <cassert>
#include <sstream>
#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef NDEBUG
#define log_do(STR, LEVEL) do { if (LEVEL > ::bitforge::Log::ReportingLevel) ; else ::bitforge::Log().Get(LEVEL) << basename(const_cast<char*>(__FILE__)) << ':' << __LINE__ << ": " << STR; } while(false)
#else
#define log_do(STR, LEVEL) do { if (LEVEL > ::bitforge::Log::ReportingLevel) ; else ::bitforge::Log().Get(LEVEL) << STR; } while(false)
#endif

#define log_info(STR) do { log_do(STR, ::bitforge::Log::logINFO); } while(false)
#define log_warning(STR) do { log_do(STR, ::bitforge::Log::logINFO); } while(false)
#define log_error(STR) do { log_do(STR, ::bitforge::Log::logINFO); } while(false)

#ifndef NDEBUG
#define log_debug(STR) do { log_do(STR, ::bitforge::Log::logDEBUG); } while(false)
#else
#define log_debug(STR) do { } while(false)
#endif

namespace bitforge
{

class Log
{
public:
	enum LogLevel
	{
		logERROR,
		logWARNING,
		logINFO,
		logDEBUG
	};

	Log(): m_os(&std::cout), m_messageLevel(ReportingLevel) {}

	~Log()
	{
		if (m_messageLevel <= Log::ReportingLevel)
		{
			(*m_os) << std::endl;
		}
	}

	std::ostream& Get(LogLevel level = logINFO)
	{
		namespace pt = boost::posix_time;
		(*m_os) << "- " << pt::to_iso_string(pt::second_clock::local_time()) << " ";

		switch(level)
		{
			case logERROR: (*m_os)		<< "ERROR   :"; break;
			case logWARNING: (*m_os)	<< "WARNING :"; break;
			case logINFO: (*m_os)		<< "INFO    :"; break;
			case logDEBUG: (*m_os)		<< "DEBUG   :"; break;
		};

		m_messageLevel = level;
		return (*m_os);
	}

   static LogLevel ReportingLevel;

protected:
	std::ostream *m_os;
	LogLevel m_messageLevel;

#if (__cplusplus >= 201103L)
public:
	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
	bool operator==(const Log&) const = delete;
#else
private:
	Log(const Log&);
	Log& operator=(const Log&);
	bool operator==(const Log&);
#endif
};

} /* namespace bitforge */

#endif /* __INCLUDE_LIBBF_LOG_H_ */
