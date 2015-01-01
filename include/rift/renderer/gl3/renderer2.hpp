#ifndef RENDERER2_HPP
#define RENDERER2_HPP

#include <array>
#include <vector>
#include <renderer_common.hpp>

// VertexLayout
#include <vertexlayout.hpp>
// Buffer
#include <buffer.hpp>
// Texture
#include <texture.hpp>
// RenderTarget
#include <rendertarget.hpp>
// Window class
#include <window.hpp> 

#include <array_ref.hpp>


class Renderer
{
public:
	Renderer(Window &window_);

	Texture2D createTexture2D(
		glm::ivec2 size,
		unsigned int numMipLevels,
		ElementFormat pixelFormat,
		const void* data);

	TextureCubeMap createTextureCubeMap(
		glm::ivec2 size,
		unsigned int numMipLevels,
		ElementFormat pixelFormat,
		const void* faceData[6]);

	VertexLayout createVertexLayout(std::array_ref<VertexElement2> vertexElements);

	Buffer createBuffer(
		std::size_t size, 
		ResourceUsage resourceUsage,
		BufferUsage bufferUsage,
		const void* initialData);

	RenderTarget createRenderTarget2D(Texture2D *texture2D, unsigned int mipLevel);
	RenderTarget createRenderTarget2D(TextureCubeMap *cubeMap, unsigned int mipLevel, unsigned int face);
	RenderTarget createRenderTargetCubeMap(TextureCubeMap *cubeMap, unsigned int mipLevel);

	void setRenderTargets(
		std::array_ref<RenderTarget*> colorTargets,
		RenderTarget *depthStencilTarget);

	// XXX all functions related to pipeline state (not. shaders) do not really make sense
	// The pipeline state interface should be hidden, all definitions should be contained
	// in an effect file, and the states should be set by an API-specific backend

private:
	void setDebugCallback();

	Window *window;
	GLuint fbo;
};

 
#endif /* end of include guard: RENDERER2_HPP */