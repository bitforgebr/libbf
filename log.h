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
#define log_do(STR, LEVEL) do { if (LEVEL > ::opm::Log::ReportingLevel) ; else ::opm::Log().Get(LEVEL) << basename(const_cast<char*>(__FILE__)) << ':' << __LINE__ << ": " << STR; } while(false)
#else
#define log_do(STR, LEVEL) do { if (LEVEL > ::opm::Log::ReportingLevel) ; else ::opm::Log().Get(LEVEL) << STR; } while(false)
#endif

#define log_info(STR) do { log_do(STR, ::opm::Log::logINFO); } while(false)
#define log_warning(STR) do { log_do(STR, ::opm::Log::logINFO); } while(false)
#define log_error(STR) do { log_do(STR, ::opm::Log::logINFO); } while(false)

#ifndef NDEBUG
#define log_debug(STR) do { log_do(STR, ::opm::Log::logDEBUG); } while(false)
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

	Log() {}
	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
	bool operator==(const Log&) const = delete;

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
	std::ostream *m_os = &std::cout;
	LogLevel m_messageLevel = ReportingLevel;
};

} /* namespace bitforge */

#endif /* __INCLUDE_LIBBF_LOG_H_ */
