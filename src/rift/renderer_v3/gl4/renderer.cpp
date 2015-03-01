// OpenGL4+ renderer implementation
#include <renderer_common.hpp>
#include <renderer.hpp>
#include <gl_common.hpp>
#include <map>
#include <string>
#include <log.hpp>

std::unique_ptr<Renderer> Renderer::instance;

namespace std {
	template <> struct hash<SamplerDesc>
	{
		size_t operator()(const SamplerDesc& v) const
		{
			auto h = hash<int>();
			return h(static_cast<int>(v.addrU))
				^ h(static_cast<int>(v.addrV))
				^ h(static_cast<int>(v.addrW))
				^ h(static_cast<int>(v.minFilter))
				^ h(static_cast<int>(v.magFilter));
		}
	};
}

// operator== for samplerDesc
bool operator==(const SamplerDesc &lhs, const SamplerDesc &rhs)
{
	return (lhs.addrU == rhs.addrU)
		&& (lhs.addrV == rhs.addrV)
		&& (lhs.addrW == rhs.addrW)
		&& (lhs.minFilter == rhs.minFilter)
		&& (lhs.magFilter == rhs.magFilter);
}

namespace gl4
{

namespace 
{
	GLenum bufferUsageToBindingPoint(BufferUsage bufferUsage)
	{
		switch (bufferUsage)
		{
		case BufferUsage::VertexBuffer:
			return gl::ARRAY_BUFFER;
		case BufferUsage::IndexBuffer:
			return gl::ELEMENT_ARRAY_BUFFER;
		case BufferUsage::ConstantBuffer:
			return gl::UNIFORM_BUFFER;
		case BufferUsage::Unspecified:
		default:
			return gl::ARRAY_BUFFER;
		}
	}

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

	void setDebugCallback()
	{
		gl::DebugMessageCallback(debugCallback, nullptr);
		gl::DebugMessageControl(gl::DONT_CARE, gl::DONT_CARE, gl::DONT_CARE, 0, nullptr, true);
		gl::DebugMessageInsert(
			gl::DEBUG_SOURCE_APPLICATION,
			gl::DEBUG_TYPE_MARKER,
			1111,
			gl::DEBUG_SEVERITY_NOTIFICATION, -1,
			"Started logging OpenGL messages");
	}


	GLenum cullModeToGLenum_[static_cast<int>(CullMode::Max)] = {
		/*None        */ gl::FRONT,	// dummy
		/*Front       */ gl::FRONT,
		/*Back        */ gl::BACK,
		/*FrontAndBack*/ gl::FRONT_AND_BACK
	};

	GLenum cullModeToGLenum(CullMode mode)
	{
		return cullModeToGLenum_[static_cast<int>(mode)];
	}

	GLenum fillModeToGLenum(PolygonFillMode fillMode)
	{
		if (fillMode == PolygonFillMode::Wireframe) {
			return gl::LINE;
		}
		else {
			return gl::FILL;
		}
	}

	GLenum primitiveTypeToGLenum_[static_cast<int>(PrimitiveType::Max)] =
	{
		/*Point        */ gl::POINTS,
		/*Line         */ gl::LINES,
		/*Triangle     */ gl::TRIANGLES,
		/*TriangleStrip*/ gl::TRIANGLE_STRIP
	};
	
	GLenum primitiveTypeToGLenum(PrimitiveType type)
	{
		return primitiveTypeToGLenum_[static_cast<int>(type)];
	}

	GLuint createBuffer(
		GLenum bindingPoint,
		int size, 
		GLuint flags, 
		const void *initialData
		)
	{
		GLuint id;
		gl::GenBuffers(1, &id);
		gl::BindBuffer(bindingPoint, id);
		// allocate immutable storage
		gl::BufferStorage(bindingPoint, size, initialData, flags);
		gl::BindBuffer(bindingPoint, 0);
		return id;
	}
}

//=============================================================================
//=============================================================================
// Renderer::createMesh
//=============================================================================
//=============================================================================
Mesh::Mesh(
	PrimitiveType primitiveType,
	std::array_ref<Attribute> layout,
	int numVertices,
	const void *vertexData,
	int numIndices,
	const void *indexData,
	std::array_ref<Submesh> submeshes_
	)
{
	mode = primitiveTypeToGLenum(primitiveType);

	// create VAO
	GLuint vao_;
	gl::GenVertexArrays(1, &vao_);
	vao = util::unique_resource<GLuint, Mesh::VAODeleter>(vao_);
	if (!gl::exts::var_EXT_direct_state_access) {
		gl::BindVertexArray(vao.get());
	}
	int offset = 0;
	for (int attribindex = 0; attribindex < layout.size(); ++attribindex)
	{
		const auto& attrib = layout[attribindex];
		const auto& fmt = getElementFormatInfoGL(attrib.format);
		if (gl::exts::var_EXT_direct_state_access) {
			gl::EnableVertexArrayAttribEXT(
				vao.get(), 
				attribindex);
			gl::VertexArrayVertexAttribFormatEXT(
				vao.get(),
				attribindex, 
				fmt.size, 
				fmt.type, 
				fmt.normalize, 
				offset);
			gl::VertexArrayVertexAttribBindingEXT(
				vao.get(),
				attribindex, 
				0);
		}
		else {
			gl::EnableVertexAttribArray(
				attribindex);
			gl::VertexAttribFormat(
				attribindex, 
				fmt.size, 
				fmt.type, 
				fmt.normalize, 
				offset);
			gl::VertexAttribBinding(
				attribindex, 
				0);
		}
		offset += getElementFormatSize(attrib.format);
	}
	if (!gl::exts::var_EXT_direct_state_access) {
		gl::BindVertexArray(0);
	}

	stride = offset;
	nbvertex = numVertices;
	vbsize = stride*numVertices;

	// VBO
	vb = util::unique_resource<GLuint, Mesh::Deleter>(
			createBuffer(
				gl::ARRAY_BUFFER, 
				vbsize,
				gl::DYNAMIC_STORAGE_BIT,
				vertexData
				));

	// IB
	ibsize = numIndices * 2;
	nbindex = numIndices;
	if (numIndices) {
		ib = util::unique_resource<GLuint, Mesh::Deleter>(
			createBuffer(
				gl::ELEMENT_ARRAY_BUFFER,
				ibsize,
				gl::DYNAMIC_STORAGE_BIT,
				indexData
				));
	}

	// copy submeshes
	submeshes = submeshes_.vec();
}

Texture2D::Texture2D(
	glm::ivec2 size_,
	int numMipLevels_,
	ElementFormat pixelFormat_,
	const void *data_
	) :
	size(size_),
	format(pixelFormat_),
	glformat(getElementFormatInfoGL(pixelFormat_).internalFormat)
{

}

void Texture2D::update(
	int mipLevel,
	glm::ivec2 offset,
	glm::ivec2 size,
	const void *data
	)
{

}

//=============================================================================
//=============================================================================
// Renderer::createEffectParameter
//=============================================================================
//=============================================================================
Parameter Effect::createParameter(
	const char *name
	)
{
	Parameter impl;
	impl.effect = this;
	impl.location = gl::GetUniformBlockIndex(program.get(), name);
	impl.binding = impl.location;
	gl::UniformBlockBinding(program.get(), impl.location, impl.location);
	return std::move(impl);
}

//=============================================================================
//=============================================================================
// Renderer::createEffectTextureParameter
//=============================================================================
//=============================================================================
TextureParameter Effect::createTextureParameter(
	const char *name
	)
{
	TextureParameter impl;
	impl.effect = this;
	impl.location = gl::GetUniformLocation(program.get(), name);
	return std::move(impl);
}

ConstantBuffer::ConstantBuffer(
	int size_,
	const void *initialData)
{
	ubo = util::unique_resource<GLuint, ConstantBuffer::Deleter>(
		createBuffer(gl::UNIFORM_BUFFER, size_, gl::DYNAMIC_STORAGE_BIT, initialData));
	size = size_;
}

//=============================================================================
//=============================================================================
// Renderer::createParameterBlock
//=============================================================================
//=============================================================================
ParameterBlock::ParameterBlock(Effect &effect_) : effect(&effect_)
{
	std::fill(ubo, ubo + kMaxUniformBufferBindings, 0);
	std::fill(ubo_offsets, ubo_offsets + kMaxUniformBufferBindings, 0);
	std::fill(ubo_sizes, ubo_sizes + kMaxUniformBufferBindings, 0);
	std::fill(textures, textures + kMaxTextureUnits, 0);
	std::fill(samplers, samplers + kMaxTextureUnits, 0);
}

//=============================================================================
//=============================================================================
// Renderer::setConstantBuffer
//=============================================================================
//=============================================================================
void ParameterBlock::setConstantBuffer(
	const Parameter &param,
	const ConstantBuffer &constantBuffer
	)
{
	//assert(param.size == constantBuffer.size);
	ubo[param.binding] = constantBuffer.ubo.get();
	ubo_offsets[param.binding] = 0;
	ubo_sizes[param.binding] = constantBuffer.size;
}

//=============================================================================
//=============================================================================
// Renderer::updateConstantBuffer
//=============================================================================
//=============================================================================
void ConstantBuffer::update(
	int offset,
	int size,
	const void *data
	)
{
	if (gl::exts::var_EXT_direct_state_access) {
		gl::NamedBufferSubDataEXT(ubo.get(), offset, size, data);
	}
	else {
		gl::BindBuffer(gl::UNIFORM_BUFFER, ubo.get());
		gl::BufferSubData(gl::UNIFORM_BUFFER, offset, size, data);
		gl::BindBuffer(gl::UNIFORM_BUFFER, 0);
	}
}

//=============================================================================
//=============================================================================
// Renderer::clearColor
//=============================================================================
//=============================================================================
void Renderer::clearColor(
	float r,
	float g,
	float b,
	float a
	)
{
	gl::ClearColor(r, g, b, a);
	gl::Clear(gl::COLOR_BUFFER_BIT);
}

//=============================================================================
//=============================================================================
// Renderer::clearDepth
//=============================================================================
//=============================================================================
void Renderer::clearDepth(
	float z
	)
{
	gl::ClearDepth(z);
	gl::Clear(gl::DEPTH_BUFFER_BIT);
}

//=============================================================================
//=============================================================================
// Renderer::setViewports
//=============================================================================
//=============================================================================
void Renderer::setViewports(
	std::array_ref<Viewport2> viewports
	)
{
	float vp[4] = {
		viewports[0].topLeftX,
		viewports[0].topLeftY,
		viewports[0].width,
		viewports[0].height
	};
	gl::ViewportIndexedfv(1, vp);
	gl::DepthRangeIndexed(0, viewports[0].minDepth, viewports[0].maxDepth);
	//gl::DepthRangeIndexed(0, 0.0f, 1.0f);

	// TODO more than 1 viewport
	/*for (int i = 0; i < viewports.size(); ++i) {
	float vp[4] = {
	viewports[i].topLeftX,
	viewports[i].topLeftY,
	viewports[i].width,
	viewports[i].height
	};
	gl::ViewportIndexedfv(i, vp);
	gl::DepthRangeIndexed(i, viewports[i].minDepth, viewports[i].maxDepth);
	}*/
}

//=============================================================================
//=============================================================================
// Renderer::setRenderTargets
//=============================================================================
//=============================================================================
void Renderer::setRenderTargets(
	std::array_ref<const RenderTarget*> colorTargets,
	const RenderTarget *depthStencilTarget
	)
{
	assert(colorTargets.size() != 0);
	// TODO hardcoded
	assert(colorTargets.size() <= 8);

	// hmmm...
	if (!colorTargets[0] || (colorTargets[0] == &screen_rt))
	{
		gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
	}
	else
	{
		gl::BindFramebuffer(gl::FRAMEBUFFER, fbo);
		for (int i = 0; i < colorTargets.size(); ++i)
		{
			auto rt = colorTargets[i];
			assert(rt != nullptr);
			if (rt->layer != -1)
				// bind all layers
				gl::FramebufferTexture(
					gl::FRAMEBUFFER, 
					gl::COLOR_ATTACHMENT0 + i, 
					rt->u.texture_2d->id.get(), 
					rt->mipLevel
					);
			else
				// bind a layer (face) of a cubemap or (TODO) a layer of a texture array
				gl::FramebufferTextureLayer(
					gl::FRAMEBUFFER, 
					gl::COLOR_ATTACHMENT0 + i, 
					rt->u.texture_2d->id.get(), 
					rt->mipLevel, 
					rt->layer
					);
		}
		gl::FramebufferTexture(
			gl::FRAMEBUFFER, 
			gl::DEPTH_ATTACHMENT, 
			depthStencilTarget->u.texture_2d->id.get(), 
			depthStencilTarget->mipLevel
			);
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
//=============================================================================
// Renderer::createRenderQueue
//=============================================================================
//=============================================================================
RenderQueue Renderer::createRenderQueue()
{
	// Nothing to do
	return RenderQueue{};
}

//=============================================================================
//=============================================================================
// Renderer::draw
//=============================================================================
//=============================================================================
void RenderQueue::draw(
	const Mesh &mesh,
	int submeshIndex,
	const Effect &effect,
	const ParameterBlock &parameterBlock,
	uint64_t sortHint
	)
{
	RenderItem item;
	item.effect = &effect;
	item.mesh = &mesh;
	item.submesh = submeshIndex;
	item.param_block = &parameterBlock;
	item.sort_key = sortHint;
	items.push_back(item);
}


void Renderer::drawItem(const RenderItem &item)
{
	// TODO multi-bind
	// bind vertex buffers
	gl::BindVertexArray(item.mesh->vao.get());
	gl::BindVertexBuffer(0, item.mesh->vb.get(), 0, item.mesh->stride);

	gl::BindBuffersRange(
		gl::UNIFORM_BUFFER,
		0,
		kMaxUniformBufferBindings,
		&item.param_block->ubo[0],
		&item.param_block->ubo_offsets[0],
		&item.param_block->ubo_sizes[0]
		);

	gl::BindTextures(0, kMaxTextureUnits, &item.param_block->textures[0]);
	gl::BindSamplers(0, kMaxTextureUnits, &item.param_block->samplers[0]);

	// shaders
	gl::UseProgram(item.effect->program.get());
	// Rasterizer
	if (item.effect->rs_state.cullMode == CullMode::None) {
		gl::Disable(gl::CULL_FACE);
	}
	else {
		gl::Enable(gl::CULL_FACE);
		gl::CullFace(cullModeToGLenum(item.effect->rs_state.cullMode));
	}
	gl::PolygonMode(gl::FRONT_AND_BACK, fillModeToGLenum(item.effect->rs_state.fillMode));

	const auto &sm = item.mesh->submeshes[item.submesh];

	if (item.mesh->ibsize != 0) {
		gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, item.mesh->ib.get());
		gl::DrawElementsBaseVertex(
			item.mesh->mode, 
			sm.numIndices,
			gl::UNSIGNED_SHORT, 
			reinterpret_cast<void*>(sm.startIndex),
			sm.startVertex
			);
	}
	else {
		gl::DrawArrays(
			item.mesh->mode,
			sm.startVertex,
			sm.numVertices
			);
	}
}

//=============================================================================
//=============================================================================
// Renderer::clearRenderQueue
//=============================================================================
//=============================================================================
void RenderQueue::clear(
	)
{
	items.clear();
}

//=============================================================================
//=============================================================================
// Renderer::submitRenderQueue
//=============================================================================
//=============================================================================
void Renderer::submitRenderQueue(
	RenderQueue &renderQueue
	)
{
	std::sort(renderQueue.items.begin(), renderQueue.items.end(), [](const RenderItem &i1, const RenderItem &i2) {
		return i1.sort_key < i2.sort_key;
	});

	for (const auto &ri : renderQueue.items) {
		drawItem(ri);
	}
}

//=============================================================================
//=============================================================================
// Renderer::initialize
//=============================================================================
//=============================================================================
void Renderer::initialize()
{
	instance = std::make_unique<Renderer>();
	setDebugCallback();
}

//=============================================================================
//=============================================================================
// Renderer::getInstance
//=============================================================================
//=============================================================================
Renderer &Renderer::getInstance()
{
	return *instance.get();
}

}	// end namespace gl4