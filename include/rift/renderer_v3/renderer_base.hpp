#ifndef RENDERER_BASE_HPP
#define RENDERER_BASE_HPP
 

#include <renderer_common.hpp>
#include <effect.hpp>
#include <parameter.hpp>
#include <constant_buffer.hpp>

template <typename Backend>
// issue a clear color command
void ClearColor(
	float r,
	float g,
	float b,
	float a
	);

template <typename Backend>
// issue a clear depth command
void ClearDepth(
	float z
	);

// set the color & depth render targets
void SetRenderTargets(
	std::array_ref<const RenderTarget*> colorTargets,
	const RenderTarget *depthStencilTarget
	)
{
	
}

void SetViewports(
	std::array_ref<Viewport2> viewports
	);

void SubmitRenderQueue(
	RenderQueueImpl &renderQueue
	);

 
#endif /* end of include guard: RENDERER_BASE_HPP */