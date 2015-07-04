#include <time.hpp>
#include <application.hpp>

namespace timer
{
	// returns the time since the start of application in seconds
	double getTime()
	{
		return GetApplication()->getTime();
	}

	// returns the last frame time in seconds
	double getDeltaTime()
	{
		return GetApplication()->getDeltaTime();
	}

	// returns the total number of frames that have passed
	unsigned long getFrameCount()
	{
		return GetApplication()->getFrameCount();
	}
}