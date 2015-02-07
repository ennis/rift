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
// Sampler
#include <sampler.hpp>
// Window class
#include <window.hpp> 
// Shader
#include <shader.hpp>

#include <array_ref.hpp>
#include <memory>

//==================================================================
// TODO extract interface
// move API-specific code in pImpl
class Renderer
{
public:
	Renderer(Window &window_);
	~Renderer();

	const RenderTarget *getScreenRenderTarget() const;
	const RenderTarget *getScreenDepthRenderTarget() const;

	//============= STATES =============

	void setVertexBuffer(
		int slot,
		const Buffer *buffer,
		int offset,
		int strides
		);

	void setIndexBuffer(
		const Buffer *indexBuffer,
		ElementFormat format
		);

	void setInputLayout(
		const VertexLayout *layout
		);

	void setConstantBuffer(
		int slot,
		const Buffer* buffer
		);

	void setShader(
		const Shader *shader
		);

	void setRasterizerState(
		RasterizerDesc desc
		);

	void setDepthStencilState(
		DepthStencilDesc desc
		);

	void setTexture(
		int texunit,
		const Texture *texture
		);

	void setSamplerState(
		int texUnit,
		const SamplerDesc &samplerDesc
		);

	//=============  RENDER TARGET COMMANDS =============

	// issue a clear color command
	void clearColor(
		float r,
		float g,
		float b,
		float a
		);

	// issue a clear depth command
	void clearDepth(
		float z
		);

	// set the color & depth render targets
	void setRenderTargets(
		std::array_ref<const RenderTarget*> colorTargets,
		const RenderTarget *depthStencilTarget
		);

	void setViewports(
		std::array_ref<Viewport2> viewports
		);

	//============= DRAW COMMANDS =============

	// submit vertices for rendering
	void draw(
		PrimitiveType primitiveType,
		int startVertex,
		int numVertices
		);

	void drawIndexed(
		PrimitiveType primitiveType,
		int startIndex,
		int numIndices,
		int baseVertex
		);

	void drawIndexedInstanced(
		PrimitiveType primitiveType,
		int baseInstance,
		int numInstances,
		int startIndex,
		int numIndices,
		int baseVertex
		);

	//============= SUBMIT =============

	// return submission index
	int createSubmission();

	// submit
	void submit(int submissionId);

private:
	class RendererImpl;
	std::unique_ptr<RendererImpl> impl;
};

 
#endif /* end of include guard: RENDERER2_HPP */