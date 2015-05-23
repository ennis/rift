#ifndef FRAME_TIME_GRAPH_HPP
#define FRAME_TIME_GRAPH_HPP

#include <vector>
#include <utils/array_ref.hpp>

struct NVGcontext;

void RenderFrameTimeGraph(NVGcontext *vg, float posX, float posY, util::array_ref<float> lastFrameTimes, unsigned lastFrameIndex);

#endif /* end of include guard: FRAME_TIME_GRAPH_HPP */