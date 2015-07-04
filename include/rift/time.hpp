#ifndef TIME_HPP
#define TIME_HPP

namespace timer
{
	// returns the time since the start of application in seconds
	double getTime();
	// returns the last frame time in seconds
	double getDeltaTime();
	// returns the total number of frames that have passed
	unsigned long getFrameCount();
}

 
#endif /* end of include guard: TIME_HPP */