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

#include <syslog.h>

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
#define log_warning(STR) do { log_do(STR, ::bitforge::Log::logWARNING); } while(false)
#define log_error(STR) do { log_do(STR, ::bitforge::Log::logERROR); } while(false)

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
        logERROR    = LOG_ERR,
        logWARNING  = LOG_WARNING,
        logINFO     = LOG_INFO,
        logDEBUG    = LOG_DEBUG
    };

    Log(): m_os(&std::clog) {}

    ~Log()
    {
        (*m_os) << std::endl;
    }

    static LogLevel ReportingLevel;
    
    std::ostream& Get(LogLevel level = logINFO);
    static void useSysLog(const char *ident);

protected:
    std::ostream *m_os;

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
