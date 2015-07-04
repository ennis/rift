#include <screen.hpp>
#include <application.hpp>

namespace screen
{
	// returns the screen width in pixels
	int getScreenWidth()
	{
		return GetApplication()->getWidth();
	}

	// returns the screen height in pixels
	int getScreenHeight()
	{
		return GetApplication()->getHeight();
	}
}