#include "bf.h"

#include <fstream>
#include <boost/regex.hpp>

namespace bitforge
{

int getNumCores()
{
	int ret = 0;

	boost::regex processor("(?<=processor\\s:\\s).*");
	std::ifstream cpuinfo("/proc/cpuinfo");
	while(cpuinfo.good())
	{
		char buffer[1024];
		cpuinfo.getline(buffer, sizeof(buffer));

		try
		{
			std::string str = buffer;
			auto it = str.begin(), end = str.end();
			boost::match_results<std::string::iterator> what;

			if (boost::regex_search(it, end, what, processor, boost::regex_constants::match_default | boost::regex_constants::match_stop))
			{
				for(int i = 0, size = what.size(); i != size; i++)
					if (what[i].matched)
					{
						ret = std::max(atoi(std::string(what[i].first, what[i].second).c_str()) + 1, ret);
					}
			}
		}
		catch(...)
		{
		}
	}

	return ret;
}

} // bitforge
