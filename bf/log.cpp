/*
 * log.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: gianni
 *
 * BitForge http://www.bitforge.com.br
 * Copyright (c) 2012 All Right Reserved,
 */

#include "log.h"

namespace bitforge
{

static bool s_useSysLog = false;
    
#ifndef NDEBUG
Log::LogLevel Log::ReportingLevel = Log::logDEBUG;
#else
Log::LogLevel Log::ReportingLevel = Log::logINFO;
#endif

class SysLogHelper : public std::basic_streambuf<char, std::char_traits<char> > 
{
public:
    explicit SysLogHelper(std::string ident, int facility)
    {
        facility_ = facility;
        strncpy(ident_, ident.c_str(), sizeof(ident_));
        ident_[sizeof(ident_)-1] = '\0';

        openlog(ident_, LOG_PID, facility_);
    }

    static SysLogHelper *instance;
    
protected:
    int sync()
    {
        if (buffer_.length()) 
        {
            syslog(Log::ReportingLevel, buffer_.c_str());
            buffer_.erase();
        }
        return 0;
    }

    int overflow(int c)
    {
        if (c != EOF) 
        {
            buffer_ += static_cast<char>(c);
        } 
        else 
        {
            sync();
        }
        return c;
    }

private:
    std::string buffer_;
    int facility_;
    char ident_[50];
};


std::ostream& Log::Get(Log::LogLevel level)
{
    if (!s_useSysLog)
    {
        namespace pt = boost::posix_time;
        (*m_os) << "- " << pt::to_iso_string(pt::second_clock::local_time()) << " ";
        
        switch(level)
        {
            case logERROR: (*m_os)      << "ERROR   :"; break;
            case logWARNING: (*m_os)    << "WARNING :"; break;
            case logINFO: (*m_os)       << "INFO    :"; break;
            case logDEBUG: (*m_os)      << "DEBUG   :"; break;
        };
    }
    
    return (*m_os);
}

void Log::useSysLog(const char *ident)
{
    s_useSysLog = true;
    std::clog.rdbuf(new SysLogHelper(ident, LOG_LOCAL0));
}

} /* namespace bitforge */
