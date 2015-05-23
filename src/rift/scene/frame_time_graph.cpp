#include <scene/frame_time_graph.hpp>
#include <rendering/opengl4/nanovg/nanovg.h>

void RenderFrameTimeGraph(NVGcontext *vg, float posX, float posY, util::array_ref<float> lastFrameTimes, unsigned lastFrameIndex)
{
	float curX = posX;
	const float barWidth = 3.0f;
	const float barHeightScale = 1000.f;
	auto numPoints = lastFrameTimes.size();
	//unsigned i = 0;
	for (unsigned ii = 0; ii < numPoints; ++ii)
	{
		nvgBeginPath(vg);
		const float barHeight = barHeightScale * lastFrameTimes[ii];
		nvgRect(vg, curX, posY - barHeight, barWidth, barHeight);
		curX += barWidth;
		if (ii == lastFrameIndex)
			nvgFillColor(vg, nvgRGBA(255, 70, 70, 255));
		else
			nvgFillColor(vg, nvgRGBA(70, 70, 255, 255));
		nvgFill(vg);
	}

}