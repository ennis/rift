#include <renderer2.hpp>
#include <log.hpp>
#include <map>

namespace
{
	const std::map<GLenum, std::string> gl_debug_source_names = {
		{ gl::DEBUG_SOURCE_API, "DEBUG_SOURCE_API" },
		{ gl::DEBUG_SOURCE_APPLICATION, "DEBUG_SOURCE_APPLICATION" },
		{ gl::DEBUG_SOURCE_OTHER, "DEBUG_SOURCE_OTHER" },
		{ gl::DEBUG_SOURCE_SHADER_COMPILER, "DEBUG_SOURCE_SHADER_COMPILER" },
		{ gl::DEBUG_SOURCE_THIRD_PARTY, "DEBUG_SOURCE_THIRD_PARTY" },
		{ gl::DEBUG_SOURCE_WINDOW_SYSTEM, "DEBUG_SOURCE_WINDOW_SYSTEM" }
	};

	const std::map<GLenum, std::string> gl_debug_type_names = {
		{ gl::DEBUG_TYPE_DEPRECATED_BEHAVIOR, "DEBUG_TYPE_DEPRECATED_BEHAVIOR" },
		{ gl::DEBUG_TYPE_ERROR, "DEBUG_TYPE_ERROR" },
		{ gl::DEBUG_TYPE_MARKER, "DEBUG_TYPE_MARKER" },
		{ gl::DEBUG_TYPE_OTHER, "DEBUG_TYPE_OTHER" },
		{ gl::DEBUG_TYPE_PERFORMANCE, "DEBUG_TYPE_PERFORMANCE" },
		{ gl::DEBUG_TYPE_POP_GROUP, "DEBUG_TYPE_POP_GROUP" },
		{ gl::DEBUG_TYPE_PORTABILITY, "DEBUG_TYPE_PORTABILITY" },
		{ gl::DEBUG_TYPE_PUSH_GROUP, "DEBUG_TYPE_PUSH_GROUP" },
		{ gl::DEBUG_TYPE_UNDEFINED_BEHAVIOR, "DEBUG_TYPE_UNDEFINED_BEHAVIOR" }
	};

	const std::map<GLenum, std::string> gl_debug_severity_names = {
		{ gl::DEBUG_SEVERITY_HIGH, "DEBUG_SEVERITY_HIGH" },
		{ gl::DEBUG_SEVERITY_LOW, "DEBUG_SEVERITY_LOW" },
		{ gl::DEBUG_SEVERITY_MEDIUM, "DEBUG_SEVERITY_MEDIUM" },
		{ gl::DEBUG_SEVERITY_NOTIFICATION, "DEBUG_SEVERITY_NOTIFICATION" }
	};

	void APIENTRY debugCallback(
		GLenum source, 
		GLenum type, 
		GLuint id, 
		GLenum severity, 
		GLsizei length, 
		const GLchar *msg, 
		const void *data)
	{
		std::string src_str = "Unknown";
		if (gl_debug_source_names.count(source)) src_str = gl_debug_source_names.at(source);
		std::string type_str = "Unknown";
		if (gl_debug_type_names.count(type)) type_str = gl_debug_type_names.at(type);
		std::string sev_str = "Unknown";
		if (gl_debug_severity_names.count(severity)) sev_str = gl_debug_severity_names.at(severity);

		LOG << "(GL debug: " << id << ", " << src_str << ", " << type_str << ", " << sev_str << ") " << msg;
	}
}

//=============================================================================
Renderer::Renderer(Window &window_) : window(&window_)
{
	setDebugCallback();
}

//=============================================================================
Texture2D Renderer::createTexture2D(
	glm::ivec2 size,
	unsigned int numMipLevels,
	ElementFormat pixelFormat,
	const void* data)
{
	auto tex = Texture2D(size, numMipLevels, pixelFormat);
	tex.update(0, { 0, 0 }, size, data);
	return std::move(tex);
}

//=============================================================================
VertexLayout Renderer::createVertexLayout(std::array_ref<VertexElement2> vertexElements)
{
	return VertexLayout(vertexElements);
}

//=============================================================================
Buffer Renderer::createBuffer(
	std::size_t size,
	ResourceUsage resourceUsage,
	BufferUsage bufferUsage,
	const void* initialData)
{
	GLenum binding;
	switch (bufferUsage)
	{
	case BufferUsage::VertexBuffer:
		binding = gl::ARRAY_BUFFER;
		break;
	case BufferUsage::IndexBuffer:
		binding = gl::ELEMENT_ARRAY_BUFFER;
		break;
	case BufferUsage::ConstantBuffer:
		binding = gl::UNIFORM_BUFFER;
		break;
	case BufferUsage::Unspecified:
	default:
		binding = gl::ARRAY_BUFFER;
		break;
	}

	return Buffer(binding, size, initialData, resourceUsage, 0);
}

//=============================================================================
RenderTarget Renderer::createRenderTarget2D(
	Texture2D *texture2D, 
	unsigned int mipLevel)
{
	return RenderTarget(texture2D, mipLevel, -1);
}

//=============================================================================
RenderTarget Renderer::createRenderTarget2D(
	TextureCubeMap *cubeMap,
	unsigned int mipLevel, 
	unsigned int face)
{
	return RenderTarget(cubeMap, mipLevel, face);
}

//=============================================================================
RenderTarget Renderer::createRenderTargetCubeMap(
	TextureCubeMap *cubeMap, 
	unsigned int mipLevel)
{
	return RenderTarget(cubeMap, mipLevel, -1);
}

//=============================================================================
void Renderer::setRenderTargets(
	std::array_ref<RenderTarget*> colorTargets,
	RenderTarget *depthStencilTarget)
{
	assert(colorTargets.size() != 0);
	// TODO hardcoded
	assert(colorTargets.size() < 8);

	if (colorTargets[0]->isNull()) 
	{
		gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
	}
	else 
	{
		gl::BindFramebuffer(gl::FRAMEBUFFER, fbo);
		for (int i = 0; i < colorTargets.size(); ++i)
		{
			auto rt = colorTargets[i];
			if (rt->layer != -1) 
				// bind all layers
				gl::FramebufferTexture(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0 + i, rt->texture->id, rt->mipLevel);
			else
				// bind a layer (face) of a cubemap or (TODO) a layer of a texture array
				gl::FramebufferTextureLayer(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0 + i, rt->texture->id, rt->mipLevel, rt->layer);
		}
		gl::FramebufferTexture(gl::FRAMEBUFFER, gl::DEPTH_ATTACHMENT, depthStencilTarget->texture->id, depthStencilTarget->mipLevel);
		// check fb completeness
		GLenum err;
		err = gl::CheckFramebufferStatus(gl::FRAMEBUFFER);
		assert(err == gl::FRAMEBUFFER_COMPLETE);
		// enable draw buffers
		static const GLenum drawBuffers[8] = {
			gl::COLOR_ATTACHMENT0,
			gl::COLOR_ATTACHMENT0 + 1,
			gl::COLOR_ATTACHMENT0 + 2,
			gl::COLOR_ATTACHMENT0 + 3,
			gl::COLOR_ATTACHMENT0 + 4,
			gl::COLOR_ATTACHMENT0 + 5,
			gl::COLOR_ATTACHMENT0 + 6,
			gl::COLOR_ATTACHMENT0 + 7
		};
		gl::DrawBuffers(colorTargets.size(), drawBuffers);
	}
}

//=============================================================================
void Renderer::setDebugCallback()
{
	if (gl::exts::var_KHR_debug) {
		gl::DebugMessageCallback(debugCallback, nullptr);
		gl::DebugMessageInsert(
			gl::DEBUG_SOURCE_APPLICATION, 
			gl::DEBUG_TYPE_MARKER, 
			1111, 
			gl::DEBUG_SEVERITY_NOTIFICATION, -1,
			"Started logging OpenGL messages");
	}
}