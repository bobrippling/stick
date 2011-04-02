#include <stdlib.h>

#include "util.h"

namespace Util
{
	void mssleep(long ms)
	{
		long when = mstime() + ms;
		while(mstime() < when);
	}
}
