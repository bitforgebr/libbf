/*
 * service.h
 *
 *  Created on: Mar 26, 2013
 *      Author: gianni
 *
 * BitForge http://www.bitforge.com.br
 * Copyright (c) 2013 All Right Reserved,
 */

#ifndef __INCLUDE_LIBBF_SERVICE_H_
#define __INCLUDE_LIBBF_SERVICE_H_

#include <bf/bf.h>
#include <unistd.h>

namespace bitforge
{

template<typename FN, typename... Arguments>
auto runAsAService(FN fn,Arguments... params) -> decltype(fn(params...))
{
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
        throw ErrnoException(strerror(errno), errno);

    /* If we got a good PID, then
        we can exit the parent process. */
    if (pid > 0)
        return decltype(fn(params...))();

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0)
        throw ErrnoException(strerror(errno), errno);

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* The Big Loop */
    return fn(params...);
}

}

#endif  // __INCLUDE_LIBBF_SERVICE_H_

