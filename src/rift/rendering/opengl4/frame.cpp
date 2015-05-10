#include <rendering/opengl4/opengl4.hpp>

namespace
{
	int frame_counter = 0;
}

namespace gl4
{
	unsigned getFrameCounter()
	{
		return frame_counter;
	}

	void beginFrame()
	{
		// reclaim transient buffers for frame n-2
		gl4::reclaimTransientBuffers();
	}

	void endFrame()
	{
		gl4::syncTransientBuffers();
		// put fences
		frame_counter++;
	}
}