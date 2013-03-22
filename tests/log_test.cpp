#include <gtest/gtest.h>

#include "../bf/log.h"

using namespace bitforge;

TEST(Logs, Syslog)
{
    Log::useSysLog("TestLog");
    
    log_info("Info");
    log_warning("warning");
    log_error("Error");
    
    log_debug("Debug");
}
