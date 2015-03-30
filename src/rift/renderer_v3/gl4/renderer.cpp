// OpenGL4+ renderer implementation
#include <renderer_common.hpp>
#include <renderer.hpp>
#include <gl_common.hpp>
#include <map>
#include <string>
#include <log.hpp>
#include <engine.hpp>
#include <array>

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

	GLenum blendOpToGL_[static_cast<int>(BlendOp::Max_)] = 
	{
		/*Add*/      gl::FUNC_ADD,
		/*Subtract*/ gl::FUNC_SUBTRACT,
		/*RevSubtract*/ gl::FUNC_REVERSE_SUBTRACT,
		/*Min*/      gl::MIN,
		/*Max*/      gl::MAX
	};

	GLenum blendOpToGL(BlendOp bo)
	{
		return blendOpToGL_[static_cast<int>(bo)];
	}

	GLenum blendFactorToGL_[static_cast<int>(BlendFactor::Max)] = {
		/*Zero*/         gl::ZERO,
		/*One*/          gl::ONE,
		/*SrcRgb*/       gl::SRC_COLOR,
		/*InvSrcRgb*/    gl::ONE_MINUS_SRC_COLOR,
		/*DestRgb*/      gl::DST_COLOR,
		/*InvDestRgb*/   gl::ONE_MINUS_DST_COLOR,
		/*SrcAlpha*/     gl::SRC_ALPHA,
		/*InvSrcAlpha*/  gl::ONE_MINUS_SRC_ALPHA,
		/*DestAlpha*/    gl::DST_ALPHA,
		/*InvDestAlpha*/ gl::ONE_MINUS_DST_ALPHA
	};

	GLenum blendFactorToGL(BlendFactor bf)
	{
		return blendFactorToGL_[static_cast<int>(bf)];
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

		if (severity != gl::DEBUG_SEVERITY_LOW && severity != gl::DEBUG_SEVERITY_NOTIFICATION)
			LOG << "(GL debug: " << id << ", " << src_str << ", " << type_str << ", " << sev_str << ") " << msg;
	}

	void setDebugCallback()
	{
		gl::Enable(gl::DEBUG_OUTPUT_SYNCHRONOUS);
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
	
	void updateBuffer(
		GLuint buf,
		GLenum target,
		int offset,
		int size,
		const void *data
		)
	{
		if (gl::exts::var_EXT_direct_state_access) {
			gl::NamedBufferSubDataEXT(buf, offset, size, data);
		}
		else {
			gl::BindBuffer(target, buf);
			gl::BufferSubData(target, offset, size, data);
		}
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


InputLayout::InputLayout(util::array_ref<Attribute> attribs)
{
	// create VAO
	gl::GenVertexArrays(1, &vao);
	if (!gl::exts::var_EXT_direct_state_access) {
		gl::BindVertexArray(vao);
	}
	int offset = 0;
	for (int attribindex = 0; attribindex < attribs.size(); ++attribindex)
	{
		const auto& attrib = attribs[attribindex];
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
}

Buffer::Buffer(
	int size_,
	ResourceUsage resourceUsage_,
	BufferUsage usage_,
	void *initialData_) :
	target(bufferUsageToBindingPoint(usage_)), size(size_)
{
	gl::GenBuffers(1, &id);
	gl::BindBuffer(target, id);
	// allocate immutable storage
	if (resourceUsage_ == ResourceUsage::Dynamic)
		flags = gl::DYNAMIC_STORAGE_BIT;
	gl::BufferStorage(target, size, initialData_, flags);
	/*if (usage_ == ResourceUsage::Dynamic)
	gl::BufferData(bindingPoint, size, data, gl::STREAM_DRAW);
	else
	gl::BufferData(bindingPoint, size, data, gl::STATIC_DRAW);*/
	gl::BindBuffer(target, 0);
}

void Buffer::update(
	int offset,
	int size,
	const void *data)
{
	if (gl::exts::var_EXT_direct_state_access) {
		gl::NamedBufferSubDataEXT(id, offset, size, data);
	}
	else {
		gl::BindBuffer(target, id);
		gl::BufferSubData(target, offset, size, data);
	}
}

Buffer::Ptr Buffer::create(
	int size,
	ResourceUsage resourceUsage,
	BufferUsage usage,
	void *initialData
	)
{
	auto ptr = std::make_unique<Buffer>(size, resourceUsage, usage, initialData);
	return ptr;
}

void Mesh::setSubmesh(int index, const Submesh &submesh)
{
	submeshes[index] = submesh;
}

void Mesh::updateVertices(int offset, int size, const void *data)
{
	updateBuffer(vb, gl::ARRAY_BUFFER, offset*stride, size*stride, data);
}

void Mesh::updateIndices(int offset, int size, const uint16_t *data)
{
	updateBuffer(ib, gl::ELEMENT_ARRAY_BUFFER, offset*2, size*2, data);
}

//=============================================================================
//=============================================================================
// Renderer::createMesh
//=============================================================================
//=============================================================================
Mesh::Mesh(
	util::array_ref<Attribute> layout,
	int numVertices,
	const void *vertexData,
	int numIndices,
	const void *indexData,
	util::array_ref<Submesh> submeshes_,
	ResourceUsage usage)
{
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

	vb_flags = 0;
	ib_flags = 0;

	if (usage == ResourceUsage::Dynamic) 
	{
		vb_flags |= gl::DYNAMIC_STORAGE_BIT;
		ib_flags |= gl::DYNAMIC_STORAGE_BIT;
	}

	// VBO
	nbvb = 1;
	vb = createBuffer(
				gl::ARRAY_BUFFER, 
				vbsize,
				vb_flags,
				vertexData
				);

	// IB
	ibsize = numIndices * 2;
	nbindex = numIndices;
	if (numIndices) {
		ib = createBuffer(
				gl::ELEMENT_ARRAY_BUFFER,
				ibsize,
				ib_flags,
				indexData
				);
	}

	submeshes.assign(submeshes_.begin(), submeshes_.end());
}

TextureCubeMap::TextureCubeMap(
	glm::ivec2 size_,
	int numMipLevels_,
	ElementFormat pixelFormat_,
	const void* faceData[6]
	) : 
	size(size_), 
	format(pixelFormat_), 
	glformat(getElementFormatInfoGL(pixelFormat_).internalFormat)
{
	gl::GenTextures(1, &id);
	const auto &pf = getElementFormatInfoGL(pixelFormat_);
	if (gl::exts::var_EXT_direct_state_access) {
		gl::TextureStorage2DEXT(id, gl::TEXTURE_CUBE_MAP, numMipLevels_, glformat, size.x, size.y);
	}
	else {
		gl::BindTexture(gl::TEXTURE_CUBE_MAP, id);
		gl::TexStorage2D(gl::TEXTURE_CUBE_MAP, numMipLevels_, glformat, size.x, size.y);
	}

	for (int i = 0; i < 6; ++i) {
		if (faceData[i])
			update(i, 0, { 0, 0 }, size, faceData[i]);
	}
}

void TextureCubeMap::update(
	int face,
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
			gl::TEXTURE_CUBE_MAP_POSITIVE_X + face,
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
		gl::BindTexture(gl::TEXTURE_CUBE_MAP, id);
		gl::TexSubImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_X + face,
			mipLevel,
			offset.x,
			offset.y,
			size.x,
			size.y,
			pf.externalFormat,
			pf.type,
			data);
		gl::BindTexture(gl::TEXTURE_CUBE_MAP, 0);
	}
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
		gl::TextureStorage2DEXT(tex, gl::TEXTURE_2D, numMipLevels_, glformat, size.x, size.y);
	}
	else {
		gl::BindTexture(gl::TEXTURE_2D, tex);
		gl::TexStorage2D(gl::TEXTURE_2D, numMipLevels_, glformat, size.x, size.y);
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

RenderTarget2::RenderTarget2() :
fbo(0),
depth_target(nullptr),
render_queue(RenderQueue::create()),
clear_depth(1.0)
{

}

namespace
{
	void checkForUnusualColorFormats(ElementFormat f)
	{
		if (f == ElementFormat::Uint8x4 ||
			f == ElementFormat::Uint8x3 ||
			f == ElementFormat::Uint8x2 ||
			f == ElementFormat::Uint8 ||
			f == ElementFormat::Uint16x2 ||
			f == ElementFormat::Uint16 ||
			f == ElementFormat::Uint32)
		{
			WARNING << "Unusual integer color format (" << getElementFormatName(f) << ") used for render target.\n";
			WARNING << "-> Did you mean to use UnormXxY ?";
		}
	}
}

RenderTarget2::RenderTarget2(
	glm::ivec2 size_,
	util::array_ref<ElementFormat> colorTargetFormats) :
	size(size_),
	render_queue(RenderQueue::create()),
	clear_depth(1.0)
{
	assert(colorTargetFormats.size() < 8);
	for (auto format : colorTargetFormats) {
		checkForUnusualColorFormats(format);
		color_targets.push_back(Texture2D::create(size, 1, format, nullptr));
	}
	init();
}

RenderTarget2::RenderTarget2(
	glm::ivec2 size_,
	util::array_ref<ElementFormat> colorTargetFormats,
	ElementFormat depthTargetFormat) :
	size(size_),
	depth_target(nullptr),
	render_queue(RenderQueue::create()),
	clear_depth(1.0)
{
	assert(colorTargetFormats.size() < 8);
	for (auto format : colorTargetFormats) {
		checkForUnusualColorFormats(format);
		color_targets.push_back(Texture2D::create(size, 1, format, nullptr));
	}
	depth_target = Texture2D::create(size, 1, depthTargetFormat, nullptr);
	init();
}

RenderTarget2::Ptr RenderTarget2::create(
	glm::ivec2 size,
	util::array_ref<ElementFormat> colorTargetFormats, 
	ElementFormat depthTargetFormat)
{
	auto ptr = std::make_unique<RenderTarget2>(size, colorTargetFormats, depthTargetFormat);
	return ptr;
}

RenderTarget2::Ptr RenderTarget2::createNoDepth(
	glm::ivec2 size, 
	util::array_ref<ElementFormat> colorTargetFormats)
{
	auto ptr = std::make_unique<RenderTarget2>(size, colorTargetFormats);
	return ptr;
}

RenderTarget2 &RenderTarget2::getDefaultRenderTarget()
{
	if (!default_rt)
		default_rt = std::make_unique<RenderTarget2>();
	return *default_rt;
}

void RenderTarget2::init()
{
	gl::GenFramebuffers(1, &fbo);
	gl::BindFramebuffer(gl::FRAMEBUFFER, fbo);

	for (int i = 0; i < color_targets.size(); ++i)
	{
		auto &&target = color_targets[i];
		gl::FramebufferTexture(
			gl::FRAMEBUFFER,
			gl::COLOR_ATTACHMENT0 + i,
			target->id,
			0);
	}
	gl::FramebufferTexture(
		gl::FRAMEBUFFER,
		gl::DEPTH_ATTACHMENT,
		depth_target->id,
		0);

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

	gl::DrawBuffers(color_targets.size(), drawBuffers);

	GLenum err;
	err = gl::CheckFramebufferStatus(gl::FRAMEBUFFER);
	assert(err == gl::FRAMEBUFFER_COMPLETE);
}

void RenderTarget2::flush()
{
	gl::BindFramebuffer(gl::FRAMEBUFFER, fbo);
	if (fbo == 0) {
		// TODO feels like a hack
		auto screen_size = Engine::instance().getWindow().size();
		gl::Viewport(0, 0, screen_size.x, screen_size.y);
	}
	else {
		gl::Viewport(0, 0, size.x, size.y);
	}
	gl::DepthMask(gl::TRUE_);
	gl::ClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
	gl::ClearDepth(clear_depth);
	gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
	Renderer::getInstance().submitRenderQueue(*render_queue);
	render_queue->clear();
}

RenderTarget2::Ptr RenderTarget2::default_rt;

namespace
{
	std::array<GLenum, 37> samplerTypes = {
		gl::SAMPLER_1D,
		gl::SAMPLER_2D,
		gl::SAMPLER_3D,
		gl::SAMPLER_CUBE,
		gl::SAMPLER_1D_SHADOW,
		gl::SAMPLER_2D_SHADOW,
		gl::SAMPLER_1D_ARRAY,
		gl::SAMPLER_2D_ARRAY,
		gl::SAMPLER_1D_ARRAY_SHADOW,
		gl::SAMPLER_2D_ARRAY_SHADOW,
		gl::SAMPLER_2D_MULTISAMPLE,
		gl::SAMPLER_2D_MULTISAMPLE_ARRAY,
		gl::SAMPLER_CUBE_SHADOW,
		gl::SAMPLER_BUFFER,
		gl::SAMPLER_2D_RECT,
		gl::SAMPLER_2D_RECT_SHADOW,
		gl::INT_SAMPLER_1D,
		gl::INT_SAMPLER_2D,
		gl::INT_SAMPLER_3D,
		gl::INT_SAMPLER_CUBE,
		gl::INT_SAMPLER_1D_ARRAY,
		gl::INT_SAMPLER_2D_ARRAY,
		gl::INT_SAMPLER_2D_MULTISAMPLE,
		gl::INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
		gl::INT_SAMPLER_BUFFER,
		gl::INT_SAMPLER_2D_RECT,
		gl::UNSIGNED_INT_SAMPLER_1D,
		gl::UNSIGNED_INT_SAMPLER_2D,
		gl::UNSIGNED_INT_SAMPLER_3D,
		gl::UNSIGNED_INT_SAMPLER_CUBE,
		gl::UNSIGNED_INT_SAMPLER_1D_ARRAY,
		gl::UNSIGNED_INT_SAMPLER_2D_ARRAY,
		gl::UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,
		gl::UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
		gl::UNSIGNED_INT_SAMPLER_BUFFER,
		gl::UNSIGNED_INT_SAMPLER_2D_RECT
	};

	bool isSamplerType(GLenum type)
	{
		for (auto e : samplerTypes) {
			if (type == e)
				return true;
		}
		return false;
	}
}

Shader::Shader(
	const char *vsSource,
	const char *psSource,
	const RasterizerDesc &rasterizerState,
	const DepthStencilDesc &depthStencilState,
	const BlendDesc &blendState)
{
	cache_id = shader_cache_index++;
	ds_state = depthStencilState;
	rs_state = rasterizerState;
	om_state = blendState;
	// preprocess source
	program = glslCreateProgram(vsSource, psSource);

	// query some informations
	int num_ubo = 0;
	gl::GetProgramInterfaceiv(program, gl::UNIFORM_BLOCK, gl::ACTIVE_RESOURCES, &num_ubo);
	GLenum bufferProps[] = { gl::BUFFER_BINDING, gl::BUFFER_DATA_SIZE };
	for (auto block_index = 0; block_index < num_ubo; ++block_index) {
		int len;
		int val[2];
		gl::GetProgramResourceiv(
			program, 
			gl::UNIFORM_BLOCK, 
			block_index, 
			2, 
			bufferProps, 
			2*sizeof(int), 
			&len, 
			val);
		LOG << "Block index #" << block_index << ": slot=" << val[0] << ", size=" << val[1];
	}

	int num_uniforms = 0;
	gl::GetProgramInterfaceiv(program, gl::UNIFORM, gl::ACTIVE_RESOURCES, &num_uniforms);
	GLenum uniformProps[] = { gl::TYPE, gl::LOCATION };
	for (auto uindex = 0; uindex < num_uniforms; ++uindex) {
		int len;
		int val[2];
		gl::GetProgramResourceiv(
			program,
			gl::UNIFORM,
			uindex,
			2,
			uniformProps,
			2 * sizeof(int),
			&len,
			val);
		LOG << "Uniform index #" << uindex << ": type=" << val[0] << ", location=" << val[1];
		if (isSamplerType(val[0])) {
			int tex_unit;
			gl::GetUniformiv(program, val[1], &tex_unit);
			LOG << " -- Sampler bound to texture unit " << tex_unit;
		}
	}
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
	num_ubo = 0;
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
	int binding,
	const ConstantBuffer &constantBuffer
	)
{
	//assert(param.size == constantBuffer.size);

	num_ubo = std::max(num_ubo, binding+1);
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

void ParameterBlock::setTextureParameter(
	int texunit,
	const TextureCubeMap *texture,
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
	int size_,
	const void *data
	)
{
	// XXX ghetto check
	if (size != size_) {
		WARNING << "Partial buffer update (" << ubo << ")";
	}

	if (gl::exts::var_EXT_direct_state_access) {
		gl::NamedBufferSubDataEXT(ubo, offset, size_, data);
	}
	else {
		gl::BindBuffer(gl::UNIFORM_BUFFER, ubo);
		gl::BufferSubData(gl::UNIFORM_BUFFER, offset, size_, data);
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
	gl::DepthMask(gl::TRUE_);
	gl::ClearDepth(z);
	gl::Clear(gl::DEPTH_BUFFER_BIT);
}

//=============================================================================
//=============================================================================
// Renderer::setViewports
//=============================================================================
//=============================================================================
void Renderer::setViewports(
	util::array_ref<Viewport2> viewports
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
	util::array_ref<const RenderTarget*> colorTargets,
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

		const GLenum noDrawBuffers = gl::NONE;

		if (colorTargets.size() == 0) {
			gl::DrawBuffers(1, &noDrawBuffers);
		}
		else {
			gl::DrawBuffers(colorTargets.size(), drawBuffers);
		}
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
	int submesh_index,
	const Shader &shader,
	const ParameterBlock &parameterBlock,
	uint64_t sortHint
	)
{
	RenderItem item;
	item.shader = &shader;
	item.mesh = &mesh;
	item.submesh_index = submesh_index;
	item.param_block = &parameterBlock;
	item.sort_key = sortHint;
	item.procedural_count = 0;
	item.num_instances = 1;
	assert(item.submesh_index < mesh.submeshes.size());
	items.push_back(item);
}

void RenderQueue::drawProcedural(
	PrimitiveType primitiveType,
	int count,
	const Shader &shader,
	const ParameterBlock &parameterBlock,
	uint64_t sortHint)
{
	RenderItem item;
	item.shader = &shader;
	item.mesh = nullptr;
	item.submesh_index = 0;
	item.param_block = &parameterBlock;
	item.sort_key = sortHint;
	item.procedural_count = count;
	item.procedural_mode = primitiveTypeToGLenum(primitiveType);
	item.num_instances = 0;
	items.push_back(item);
}

void RenderQueue::drawInstanced(
	const Mesh &mesh,
	int submesh_index,
	const Shader &shader,
	const ParameterBlock &parameterBlock,
	int num_instances,
	uint64_t sortHint
	)
{
	RenderItem item;
	item.shader = &shader;
	item.mesh = &mesh;
	item.submesh_index = submesh_index;
	item.param_block = &parameterBlock;
	item.sort_key = sortHint;
	item.procedural_count = 0;
	item.num_instances = num_instances;
	assert(item.submesh_index < mesh.submeshes.size());
	items.push_back(item);
}

void Renderer::drawItem(const RenderItem &item)
{
	// TODO multi-bind
	// bind vertex buffers
	// mesh input
	if (item.mesh)
	{
		gl::BindVertexArray(item.mesh->vao);
		gl::BindVertexBuffer(0, item.mesh->vb, 0, item.mesh->stride);
	}
	else
	{
		// fully procedural (no VBO)
		gl::BindVertexArray(dummy_vao);
	}

	gl::BindBuffersRange(
		gl::UNIFORM_BUFFER,
		0,
		item.param_block->num_ubo,
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
	gl::Enable(gl::DEPTH_TEST);
	if (!item.shader->ds_state.depthTestEnable)
		gl::DepthFunc(gl::ALWAYS);
	else
		gl::DepthFunc(gl::LEQUAL);
	if (item.shader->ds_state.depthWriteEnable)
		gl::DepthMask(gl::TRUE_);
	else
		gl::DepthMask(gl::FALSE_);

	// OM / blend state
	// XXX this ain't cheap
	// TODO blend state per color buffer
	gl::Enable(gl::BLEND);
	gl::BlendEquationSeparatei(
		0,
		blendOpToGL(item.shader->om_state.rgbOp),
		blendOpToGL(item.shader->om_state.alphaOp));
	gl::BlendFuncSeparatei(
		0,
		blendFactorToGL(item.shader->om_state.rgbSrcFactor),
		blendFactorToGL(item.shader->om_state.rgbDestFactor),
		blendFactorToGL(item.shader->om_state.alphaSrcFactor),
		blendFactorToGL(item.shader->om_state.alphaDestFactor));

	if (item.mesh)
	{
		const auto &sm = item.mesh->submeshes[item.submesh_index];

		if (item.mesh->ibsize != 0) {
			gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, item.mesh->ib);
			if (item.num_instances != 1)
			{
				gl::DrawElementsBaseVertex(
					primitiveTypeToGLenum(sm.primitiveType),
					sm.numIndices,
					gl::UNSIGNED_SHORT,
					reinterpret_cast<void*>(sm.startIndex * 2),
					sm.startVertex
					);
			}
			else {
				gl::DrawElementsInstancedBaseVertex(
					primitiveTypeToGLenum(sm.primitiveType), 
					sm.numIndices, 
					gl::UNSIGNED_SHORT,
					reinterpret_cast<void*>(sm.startIndex * 2), 
					item.num_instances, 
					sm.startVertex);
			}
		}
		else {
			gl::DrawArrays(
				primitiveTypeToGLenum(sm.primitiveType),
				sm.startVertex,
				sm.numVertices
				);
		}
	}
	else
	{
		gl::DrawArrays(item.procedural_mode, 0, item.procedural_count);
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
	gl::GenVertexArrays(1, &dummy_vao);
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