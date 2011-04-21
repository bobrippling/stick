#undef NDEBUG
#include <cstdio>
#include <cassert>

int main(void)
{
	assert((puts("checked sizeof(int)"),   sizeof(int)   == 4));
	assert((puts("checked sizeof(float)"), sizeof(float) == 4));
	return 0;
}
