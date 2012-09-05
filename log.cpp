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

#ifndef NDEBUG
Log::LogLevel Log::ReportingLevel = Log::logDEBUG;
#else
Log::LogLevel Log::ReportingLevel = Log::logINFO;
#endif

} /* namespace bitforge */
