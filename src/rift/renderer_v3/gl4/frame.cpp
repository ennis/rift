#include <renderer.hpp>

namespace
{
	int frame_counter = 0;
}


namespace Renderer
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