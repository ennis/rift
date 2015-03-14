// OpenGL4+ renderer implementation
#include <renderer_common.hpp>
#include <renderer.hpp>
#include <gl_common.hpp>
#include <map>
#include <string>
#include <log.hpp>

std::unique_ptr<Renderer> Renderer::instance;


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
	GLint textureFilterToGL_[static_cast<int>(TextureFilter::Max)] = {
		/* Nearest */ gl::NEAREST,
		/* Linear */  gl::LINEAR
	};

	GLint textureAddressModeToGL_[static_cast<int>(TextureAddressMode::Max)] = {
		/* Repeat */ gl::REPEAT,
		/* Mirror */ gl::MIRRORED_REPEAT,
		/* Clamp  */ gl::CLAMP_TO_EDGE
	};

	GLint textureFilterToGL(TextureFilter filter) {
		return textureFilterToGL_[static_cast<int>(filter)];
	}

	GLint textureAddressModeToGL(TextureAddressMode addr) {
		return textureAddressModeToGL_[static_cast<int>(addr)];
	}

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

	//=========================================================================
	//=========================================================================
	// GLSL
	//=========================================================================
	//=========================================================================
	GLuint glslCompileShader(const char *shaderSource, GLenum type)
	{
		GLuint obj = gl::CreateShader(type);
		const char *shaderSources[1] = { shaderSource };
		gl::ShaderSource(obj, 1, shaderSources, NULL);
		gl::CompileShader(obj);

		GLint status = gl::TRUE_;
		GLint logsize = 0;

		gl::GetShaderiv(obj, gl::COMPILE_STATUS, &status);
		gl::GetShaderiv(obj, gl::INFO_LOG_LENGTH, &logsize);
		if (status != gl::TRUE_) {
			ERROR << "Compile error:";
			if (logsize != 0) {
				char *logbuf = new char[logsize];
				gl::GetShaderInfoLog(obj, logsize, &logsize, logbuf);
				ERROR << logbuf;
				delete[] logbuf;
				gl::DeleteShader(obj);
			}
			else {
				ERROR << "<no log>";
			}
			throw std::runtime_error("shader compilation failed");
		}

		return obj;
	}

	void glslLinkProgram(GLuint program)
	{
		GLint status = gl::TRUE_;
		GLint logsize = 0;

		gl::LinkProgram(program);
		gl::GetProgramiv(program, gl::LINK_STATUS, &status);
		gl::GetProgramiv(program, gl::INFO_LOG_LENGTH, &logsize);
		if (status != gl::TRUE_) {
			ERROR << "Link error:";
			if (logsize != 0) {
				char *logbuf = new char[logsize];
				gl::GetProgramInfoLog(program, logsize, &logsize, logbuf);
				ERROR << logbuf;
				delete[] logbuf;
			}
			else {
				ERROR << "<no log>";
			}
			throw std::runtime_error("link failed");
		}
	}

	// creates a shader program from vertex and fragment shader source files
	GLuint glslCreateProgram(const char *vertexShaderSource, const char *fragmentShaderSource)
	{
		GLuint vs_obj = glslCompileShader(vertexShaderSource, gl::VERTEX_SHADER);
		GLuint fs_obj = glslCompileShader(fragmentShaderSource, gl::FRAGMENT_SHADER);
		GLuint program_obj = gl::CreateProgram();
		gl::AttachShader(program_obj, vs_obj);
		gl::AttachShader(program_obj, fs_obj);
		glslLinkProgram(program_obj);
		// once the program is linked, no need to keep the shader objects
		gl::DetachShader(program_obj, vs_obj);
		gl::DetachShader(program_obj, fs_obj);
		gl::DeleteShader(vs_obj);
		gl::DeleteShader(fs_obj);
		return program_obj;
	}

	int shader_cache_index = 0;
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
	vao = vao_;
	if (!gl::exts::var_EXT_direct_state_access) {
		gl::BindVertexArray(vao);
	}
	int offset = 0;
	for (int attribindex = 0; attribindex < layout.size(); ++attribindex)
	{
		const auto& attrib = layout[attribindex];
		const auto& fmt = getElementFormatInfoGL(attrib.format);
		if (gl::exts::var_EXT_direct_state_access) {
			gl::EnableVertexArrayAttribEXT(
				vao, 
				attribindex);
			gl::VertexArrayVertexAttribFormatEXT(
				vao,
				attribindex, 
				fmt.size, 
				fmt.type, 
				fmt.normalize, 
				offset);
			gl::VertexArrayVertexAttribBindingEXT(
				vao,
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
	nbvb = 1;
	vb = createBuffer(
				gl::ARRAY_BUFFER, 
				vbsize,
				gl::DYNAMIC_STORAGE_BIT,
				vertexData
				);

	// IB
	ibsize = numIndices * 2;
	nbindex = numIndices;
	if (numIndices) {
		ib = createBuffer(
				gl::ELEMENT_ARRAY_BUFFER,
				ibsize,
				gl::DYNAMIC_STORAGE_BIT,
				indexData
				);
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
	GLuint tex;
	gl::GenTextures(1, &tex);
	const auto &pf = getElementFormatInfoGL(pixelFormat_);
	if (gl::exts::var_EXT_direct_state_access) {
		gl::TextureStorage2DEXT(tex, gl::TEXTURE_2D, numMipLevels_, pf.internalFormat, size.x, size.y);
	}
	else {
		gl::BindTexture(gl::TEXTURE_2D, tex);
		gl::TexStorage2D(gl::TEXTURE_2D, numMipLevels_, pf.internalFormat, size.x, size.y);
		gl::BindTexture(gl::TEXTURE_2D, 0);
	}
	
	id = tex;

	if (data_)
		update(0, { 0, 0 }, size, data_);
}


void Texture2D::update(
	int mipLevel,
	glm::ivec2 offset,
	glm::ivec2 size,
	const void *data
	)
{
	const auto &pf = getElementFormatInfoGL(format);
	if (gl::exts::var_EXT_direct_state_access)
	{
		gl::TextureSubImage2DEXT(
			id,
			gl::TEXTURE_2D,
			mipLevel,
			offset.x,
			offset.y,
			size.x,
			size.y,
			pf.externalFormat,
			pf.type,
			data);
	}
	else
	{
		gl::BindTexture(gl::TEXTURE_2D, id);
		gl::TexSubImage2D(
			gl::TEXTURE_2D,
			mipLevel,
			offset.x,
			offset.y,
			size.x,
			size.y,
			pf.externalFormat,
			pf.type,
			data);
		gl::BindTexture(gl::TEXTURE_2D, 0);
	}
}

RenderTarget::Ptr RenderTarget::createRenderTarget2D(
	Texture2D &texture2D,
	int mipLevel
	)
{
	auto r = std::make_unique<RenderTarget>();
	r->type = RenderTarget::kRenderToTexture2D;
	r->layer = 0;
	r->mipLevel = mipLevel;
	r->u.texture_2d = &texture2D;
	return r;
}

RenderTarget::Ptr RenderTarget::createRenderTarget2DFace(
	TextureCubeMap &cubeMap,
	int mipLevel,
	int face
	)
{
	auto r = std::make_unique<RenderTarget>();
	r->type = RenderTarget::kRenderToCubeMapLayer;
	r->layer = face;
	r->mipLevel = mipLevel;
	r->u.texture_cubemap = &cubeMap;
	return r;
}

RenderTarget::Ptr RenderTarget::createRenderTargetCubeMap(
	TextureCubeMap &cubeMap,
	int mipLevel
	)
{
	auto r = std::make_unique<RenderTarget>();
	r->type = RenderTarget::kRenderToCubeMap;
	r->layer = 0;
	r->mipLevel = mipLevel;
	r->u.texture_cubemap = &cubeMap;
	return r;
}

Shader::Shader(
	const char *vsSource,
	const char *psSource,
	const RasterizerDesc &rasterizerState,
	const DepthStencilDesc &depthStencilState)
{
	cache_id = shader_cache_index++;
	ds_state = depthStencilState;
	rs_state = rasterizerState;
	// preprocess source
	program = glslCreateProgram(vsSource, psSource);
}

//=============================================================================
//=============================================================================
// Renderer::createEffectParameter
//=============================================================================
//=============================================================================
Parameter::Ptr Shader::createParameter(
	const char *name
	)
{
	auto ptr = std::make_unique<Parameter>();
	ptr->shader = this;
	ptr->location = gl::GetUniformBlockIndex(program, name);
	ptr->binding = ptr->location;
	gl::UniformBlockBinding(program, ptr->location, ptr->location);
	return std::move(ptr);
}

//=============================================================================
//=============================================================================
// Renderer::createEffectTextureParameter
//=============================================================================
//=============================================================================
TextureParameter::Ptr Shader::createTextureParameter(
	const char *name
	)
{
	auto ptr = std::make_unique<TextureParameter>();
	ptr->shader = this;
	ptr->location = gl::GetUniformLocation(program, name);
	return std::move(ptr);
}

TextureParameter::Ptr Shader::createTextureParameter(
	int texunit
	)
{
	auto ptr = std::make_unique<TextureParameter>();
	ptr->shader = this;
	ptr->location = -1;
	ptr->texunit = texunit;
	return std::move(ptr);
}

ConstantBuffer::ConstantBuffer(
	int size_,
	const void *initialData)
{
	ubo = createBuffer(gl::UNIFORM_BUFFER, size_, gl::DYNAMIC_STORAGE_BIT, initialData);
	size = size_;
}

//=============================================================================
//=============================================================================
// Renderer::createParameterBlock
//=============================================================================
//=============================================================================
ParameterBlock::ParameterBlock(Shader &shader_) : shader(&shader_)
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
	ubo[param.binding] = constantBuffer.ubo;
	ubo_offsets[param.binding] = 0;
	ubo_sizes[param.binding] = constantBuffer.size;
}

void ParameterBlock::setTextureParameter(
	const TextureParameter &param,
	const Texture2D *texture,
	const SamplerDesc &samplerDesc
	)
{
	textures[param.texunit] = texture->id;
	samplers[param.texunit] = Renderer::getInstance().getSampler(samplerDesc);
}

void ParameterBlock::setConstantBuffer(
	int binding,
	const ConstantBuffer &constantBuffer
	)
{
	//assert(param.size == constantBuffer.size);
	ubo[binding] = constantBuffer.ubo;
	ubo_offsets[binding] = 0;
	ubo_sizes[binding] = constantBuffer.size;
}

void ParameterBlock::setTextureParameter(
	int texunit,
	const Texture2D *texture,
	const SamplerDesc &samplerDesc
	)
{
	textures[texunit] = texture->id;
	samplers[texunit] = Renderer::getInstance().getSampler(samplerDesc);
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
		gl::NamedBufferSubDataEXT(ubo, offset, size, data);
	}
	else {
		gl::BindBuffer(gl::UNIFORM_BUFFER, ubo);
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
	// TODO hardcoded
	assert(colorTargets.size() <= 8);

	// hmmm...
	if (((colorTargets.size() == 0) && !depthStencilTarget) || (((colorTargets.size() != 0) && colorTargets[0] == &screen_rt)))
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
					rt->u.texture_2d->id,
					rt->mipLevel
				);
			else
				// bind a layer (face) of a cubemap or (TODO) a layer of a texture array
				gl::FramebufferTextureLayer(
					gl::FRAMEBUFFER,
					gl::COLOR_ATTACHMENT0 + i,
					rt->u.texture_2d->id,
					rt->mipLevel,
					rt->layer
				);
		}
		gl::FramebufferTexture(
			gl::FRAMEBUFFER, 
			gl::DEPTH_ATTACHMENT, 
			depthStencilTarget->u.texture_2d->id, 
			depthStencilTarget->mipLevel
			);
		// check fb completeness
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
		GLenum err;
		err = gl::CheckFramebufferStatus(gl::FRAMEBUFFER);
		assert(err == gl::FRAMEBUFFER_COMPLETE);
	}
}

//=============================================================================
//=============================================================================
// Renderer::draw
//=============================================================================
//=============================================================================
void RenderQueue::draw(
	const Mesh &mesh,
	int submeshIndex,
	const Shader &shader,
	const ParameterBlock &parameterBlock,
	uint64_t sortHint
	)
{
	RenderItem item;
	item.shader = &shader;
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
	gl::BindVertexArray(item.mesh->vao);
	gl::BindVertexBuffer(0, item.mesh->vb, 0, item.mesh->stride);

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
	gl::UseProgram(item.shader->program);
	// Rasterizer
	if (item.shader->rs_state.cullMode == CullMode::None) {
		gl::Disable(gl::CULL_FACE);
	}
	else {
		gl::Enable(gl::CULL_FACE);
		gl::CullFace(cullModeToGLenum(item.shader->rs_state.cullMode));
	}
	gl::PolygonMode(gl::FRONT_AND_BACK, fillModeToGLenum(item.shader->rs_state.fillMode));
	if (item.shader->ds_state.depthTestEnable)
		gl::Enable(gl::DEPTH_TEST);
	else
		gl::Disable(gl::DEPTH_TEST);
	if (item.shader->ds_state.depthWriteEnable)
		gl::DepthMask(true);
	else
		gl::DepthMask(false);

	const auto &sm = item.mesh->submeshes[item.submesh];

	if (item.mesh->ibsize != 0) {
		gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, item.mesh->ib);
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

GLuint Renderer::getSampler(SamplerDesc desc)
{
	auto &ins = sampler_cache.insert(
		std::pair<
		SamplerDesc,
		util::unique_resource<GLuint, SamplerDeleter> >(desc, 0));
	auto &res = ins.first;
	if (ins.second) {
		GLuint id;
		gl::GenSamplers(1, &id);
		gl::SamplerParameteri(id, gl::TEXTURE_MIN_FILTER, textureFilterToGL(desc.minFilter));
		gl::SamplerParameteri(id, gl::TEXTURE_MAG_FILTER, textureFilterToGL(desc.magFilter));
		gl::SamplerParameteri(id, gl::TEXTURE_WRAP_R, textureAddressModeToGL(desc.addrU));
		gl::SamplerParameteri(id, gl::TEXTURE_WRAP_S, textureAddressModeToGL(desc.addrV));
		gl::SamplerParameteri(id, gl::TEXTURE_WRAP_T, textureAddressModeToGL(desc.addrW));
		res->second = util::unique_resource<GLuint, SamplerDeleter>(id);
	}
	return res->second.get();
}

Renderer::Renderer()
{
	gl::GenFramebuffers(1, &fbo);
}

Renderer::~Renderer()
{
	gl::DeleteFramebuffers(1, &fbo);
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