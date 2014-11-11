#include "clock.hpp"

#ifdef _MSC_VER

#include <stdexcept>
#include <string>

qpc_clock::time_point qpc_clock::now()
{
	if (!is_inited) {
		init();
		is_inited = true;
	}
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return time_point(duration(static_cast<rep>((double)counter.QuadPart / frequency.QuadPart *
		period::den / period::num)));
}

bool qpc_clock::is_inited = false;
LARGE_INTEGER qpc_clock::frequency;

void qpc_clock::init()
{
	if (QueryPerformanceFrequency(&frequency) == 0)
		throw std::logic_error("QueryPerformanceCounter not supported: " + std::to_string(GetLastError()));
}

#endif