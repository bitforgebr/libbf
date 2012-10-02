#include "bf.h"

#include <fstream>

#include <cerrno>
#include <cstring>
#include <cstdio>

namespace bitforge
{

int getNumCores()
{
	int ret = 0;

	ProcStreams getconf;
	const char* const args[] = {"getconf", "_NPROCESSORS_ONLN"};
	if (runAttachedProcess(&getconf, args))
	{
		struct timeval timeOut = {0, 500};
		int maxFD = std::max(getconf.stdOut, getconf.stdErr);
		fd_set fd;
		FD_SET(getconf.stdOut, &fd);
		FD_SET(getconf.stdErr, &fd);

		int rc = select(maxFD + 1, &fd, nullptr, nullptr, &timeOut);
		if(rc > 0 && FD_ISSET(getconf.stdOut, &fd))
		{
			std::size_t buffSize = 256;
			char buff[buffSize];
			auto r = read(getconf.stdOut, buff, buffSize);
			if (r > 0)
			{
				buff[r] = 0;
				ret = atoi(buff);
			}
		}
	}

	return ret;
}

bool runAttachedProcess(ProcStreams *streams, const char* const args[], const char* const env[])
{
	int pipeIn[2], pipeOut[2], pipeErr[2]; /* pipe */

	pipe(pipeIn);
	pipe(pipeOut);
	pipe(pipeErr);

	switch(fork())
	{
		case -1: // Error
		{
			std::cerr << "Forking error: " << strerror(errno) << std::endl;
			return false;
		}

		case 0: // child
		{
			/* stdin/stdout/stderr are replaced by pipes */
			dup2 (pipeOut[0], fileno(stdin));
			dup2 (pipeIn[1], fileno(stdout));
			dup2 (pipeErr[1], fileno(stderr));

			/* close part of pipe unneeded */
			close (pipeOut[1]);
			close (pipeIn[0]);
			close (pipeErr[0]);

			execvpe(args[0], const_cast<char* const*>(args), const_cast<char* const*>(env));

			// execvpe should never return; if it does, its an error
			std::cerr << "Forking error: " << strerror(errno) << std::endl;
			return false;
		}

		default: // parent
		{
			/* close part of pipe unneeded */
			close(pipeOut[0]);
			close(pipeIn[1]);
			close(pipeErr[1]);

			streams->stdIn = pipeOut[1];
			streams->stdOut = pipeIn[0];
			streams->stdErr = pipeErr[0];

			return true;
		}
	}
}

/* Fast hash function */
uint32_t fletcher32( uint16_t *data, ::std::size_t len )
{
	uint32_t sum1 = 0xffff, sum2 = 0xffff;

	while (len)
	{
		unsigned tlen = len > 360 ? 360 : len;
		len -= tlen;
		do
		{
			sum1 += *data++;
			sum2 += sum1;
		}
		while (--tlen);

		sum1 = (sum1 & 0xffff) + (sum1 >> 16);
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	}

	/* Second reduction step to reduce sums to 16 bits */
	sum1 = (sum1 & 0xffff) + (sum1 >> 16);
	sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	return sum2 << 16 | sum1;
}

} // bitforge
