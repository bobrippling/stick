#ifndef UTIL_H
#define UTIL_H

#ifdef _WIN32
# include <windows.h>
#else
# include <sys/time.h>
#endif

namespace Util
{
	void mssleep(long ms);

	inline long mstime(void) // current time in ms
	{
#ifdef _WIN32
		return GetTickCount();
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_usec / 1000 + tv.tv_sec * 1000;
#endif
	}
}

#endif
