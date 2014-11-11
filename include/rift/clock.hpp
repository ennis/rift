#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <chrono>

#ifdef _MSC_VER

#include <Windows.h>

// Thanks!
//http://stackoverflow.com/questions/13263277/difference-between-stdsystem-clock-and-stdsteady-clock
// Self-made Windows QueryPerformanceCounter based C++11 API compatible clock
struct qpc_clock {
	typedef std::chrono::nanoseconds                       duration;      // nanoseconds resolution
	typedef duration::rep                                  rep;
	typedef duration::period                               period;
	typedef std::chrono::time_point<qpc_clock, duration>   time_point;
	static bool is_steady;                                                // = true
	static time_point now();

private:
	static bool is_inited;                                                // = false
	static LARGE_INTEGER frequency;
	static void init();
};

#else
typedef std::chrono::steady_clock qpc_clock;
#endif

#endif