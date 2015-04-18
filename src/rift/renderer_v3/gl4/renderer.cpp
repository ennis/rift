// OpenGL4+ renderer implementation
#include <renderer_common.hpp>
#include <renderer.hpp>
#include <gl_common.hpp>
#include <map>
#include <string>
#include <log.hpp>
#include <engine.hpp>
#include <array>
#include <algorithm>

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

namespace detail
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

		//if (severity != gl::DEBUG_SEVERITY_LOW && severity != gl::DEBUG_SEVERITY_NOTIFICATION)
			LOG << "(GL debug: " << id << ", " << src_str << ", " << type_str << ", " << sev_str << ") " << msg;
	}

	void setDebugCallback()
	{
		//gl::Enable(gl::DEBUG_OUTPUT_SYNCHRONOUS);
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


InputLayout::InputLayout(unsigned num_buffers,util::array_ref<Attribute> attribs)
{
	strides.resize(num_buffers);
	std::fill(strides.begin(), strides.end(), 0);
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
				strides[attrib.inputSlot]);
			gl::VertexArrayVertexAttribBindingEXT(
				vao,
				attribindex,
				attrib.inputSlot);
		}
		else {
			gl::EnableVertexAttribArray(
				attribindex);
			gl::VertexAttribFormat(
				attribindex,
				fmt.size,
				fmt.type,
				fmt.normalize,
				strides[attrib.inputSlot]);
			gl::VertexAttribBinding(
				attribindex,
				attrib.inputSlot);
		}
		strides[attrib.inputSlot] += getElementFormatSize(attrib.format);
	}
	if (!gl::exts::var_EXT_direct_state_access) {
		gl::BindVertexArray(0);
	}
}

Buffer::Buffer(
	size_t size_,
	ResourceUsage resourceUsage_,
	BufferUsage usage_,
	const void *initialData_) :
	target(detail::bufferUsageToBindingPoint(usage_)), size(size_), flags(0)
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
	size_t offset,
	size_t size,
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
	size_t size,
	ResourceUsage resourceUsage,
	BufferUsage usage,
	const void *initialData
	)
{
	auto ptr = std::make_unique<Buffer>(size, resourceUsage, usage, initialData);
	return ptr;
}

//=============================================================================
//=============================================================================
// Renderer::createMesh
//=============================================================================
//=============================================================================
/*Mesh::Mesh(
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
	vb = detail::createBuffer(
				gl::ARRAY_BUFFER, 
				vbsize,
				vb_flags,
				vertexData
				);

	// IB
	ibsize = numIndices * 2;
	nbindex = numIndices;
	if (numIndices) {
		ib = detail::createBuffer(
				gl::ELEMENT_ARRAY_BUFFER,
				ibsize,
				ib_flags,
				indexData
				);
	}

	submeshes.assign(submeshes_.begin(), submeshes_.end());
}*/

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


RenderTarget::RenderTarget() :
fbo(0),
depth_target(nullptr),
clear_depth(1.0)
{
}

RenderTarget::RenderTarget(
	glm::ivec2 size_,
	util::array_ref<ElementFormat> colorTargetFormats) :
	size(size_),
	clear_depth(1.0)
{
	assert(colorTargetFormats.size() < 8);
	for (auto format : colorTargetFormats) {
		detail::checkForUnusualColorFormats(format);
		color_targets.push_back(Texture2D::create(size, 1, format, nullptr));
	}
	init();
}

RenderTarget::RenderTarget(
	glm::ivec2 size_,
	util::array_ref<ElementFormat> colorTargetFormats,
	ElementFormat depthTargetFormat) :
	size(size_),
	depth_target(nullptr),
	clear_depth(1.0)
{
	assert(colorTargetFormats.size() < 8);
	for (auto format : colorTargetFormats) {
		detail::checkForUnusualColorFormats(format);
		color_targets.push_back(Texture2D::create(size, 1, format, nullptr));
	}
	depth_target = Texture2D::create(size, 1, depthTargetFormat, nullptr);
	init();
}

RenderTarget::Ptr RenderTarget::create(
	glm::ivec2 size,
	util::array_ref<ElementFormat> colorTargetFormats, 
	ElementFormat depthTargetFormat)
{
	auto ptr = std::make_unique<RenderTarget>(size, colorTargetFormats, depthTargetFormat);
	return ptr;
}

RenderTarget::Ptr RenderTarget::createNoDepth(
	glm::ivec2 size, 
	util::array_ref<ElementFormat> colorTargetFormats)
{
	auto ptr = std::make_unique<RenderTarget>(size, colorTargetFormats);
	return ptr;
}

RenderTarget &RenderTarget::getDefaultRenderTarget()
{
	if (!default_rt)
		default_rt = std::make_unique<RenderTarget>();
	return *default_rt;
}

void RenderTarget::init()
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

void RenderTarget::commit(RenderQueue &renderQueue)
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
	Renderer::getInstance().commit(renderQueue);
}

RenderTarget::Ptr RenderTarget::default_rt;

Shader::Shader(
	const char *vsSource,
	const char *psSource,
	const RasterizerDesc &rasterizerState,
	const DepthStencilDesc &depthStencilState,
	const BlendDesc &blendState)
{
	cache_id = detail::shader_cache_index++;
	ds_state = depthStencilState;
	rs_state = rasterizerState;
	om_state = blendState;
	// preprocess source
	program = detail::glslCreateProgram(vsSource, psSource);

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
		if (detail::isSamplerType(val[0])) {
			int tex_unit;
			gl::GetUniformiv(program, val[1], &tex_unit);
			LOG << " -- Sampler bound to texture unit " << tex_unit;
		}
	}
}


Stream::Stream(BufferUsage usage_, size_t size_, unsigned num_buffers)
{
	// align size
	buffer_size = (size_ + 256u - 1) & ~((size_t)256u - 1);
	auto total_size = buffer_size * num_buffers;
	ranges.resize(num_buffers);
	LOG << "Allocating stream of size " << buffer_size << "x" << num_buffers;
	gl::GenBuffers(1, &buffer_object);
	buffer_target = gl4::detail::bufferUsageToBindingPoint(usage_);
	gl::BindBuffer(buffer_target, buffer_object);
	gl::BufferStorage(buffer_target, total_size, nullptr, gl::MAP_WRITE_BIT | gl::MAP_PERSISTENT_BIT | gl::MAP_COHERENT_BIT);
	gl::BindBuffer(buffer_target, 0);
	// init ranges
	for (auto i = 0u; i < num_buffers; ++i)
	{
		ranges[i].sync = nullptr;
	}
	current_offset = 0;
	current_size = 0;
	current_range = 0;
	mapped_ptr = gl::MapNamedBufferRangeEXT(buffer_object, 0, buffer_size,
		gl::MAP_INVALIDATE_BUFFER_BIT | // discard old contents
		gl::MAP_PERSISTENT_BIT | // persistent mapping
		gl::MAP_COHERENT_BIT | // coherent
		gl::MAP_WRITE_BIT | // write-only (no readback)
		gl::MAP_UNSYNCHRONIZED_BIT // no driver sync (we handle this)
		);
}

/*void Stream::remap()
{
mapped_ptr = gl::MapNamedBufferRangeEXT(buffer_object, 0, buffer_size,
gl::MAP_INVALIDATE_BUFFER_BIT | // discard old contents
gl::MAP_PERSISTENT_BIT | // persistent mapping
gl::MAP_COHERENT_BIT | // coherent
gl::MAP_WRITE_BIT | // write-only (no readback)
gl::MAP_UNSYNCHRONIZED_BIT // no driver sync (we handle this)
);
}*/

void *Stream::reserve(size_t size)
{
	assert(size <= buffer_size);

	if (size == 0) {
		return nullptr;
	}

	// still have a fence?
	if (ranges[current_range].sync != nullptr) {
		// yes, synchronize
		auto result = gl::ClientWaitSync(ranges[current_range].sync, gl::SYNC_FLUSH_COMMANDS_BIT, 0);
		if (result == gl::TIMEOUT_EXPIRED) {
			// We want absolutely no stalls
			// TODO handle this?
			assert(!"Buffer stall");
		}
		// Ok, the buffer is unused
		ranges[current_range].sync = nullptr;
	}

	// ewww
	auto ptr = reinterpret_cast<void*>(current_offset + current_size + 256);
	auto space = buffer_size - (current_offset + current_size);
	if (!std::align(256, size, ptr, space)) {
		ERROR << "Out of space in current buffer range";
		assert(false);
	}
	// buffer is unlocked and has enough space: reserve
	current_offset = buffer_size - space;
	// TODO align allocated space!!
	current_size = size;
	return reinterpret_cast<char*>(mapped_ptr)+current_range * buffer_size + current_offset;
}

void Stream::fence(RenderQueue &renderQueue)
{
	// go to the next buffer
	renderQueue.fenceSync(ranges[current_range].sync);
	current_range = (current_range + 1) % ranges.size();
	current_offset = 0;
	current_size = 0;
}

void RenderQueue::beginCommand()
{
	std::memset(&state, 0, sizeof(state));
}

void RenderQueue::setVertexBuffers(
	util::array_ref<BufferDesc> vertex_buffers,
	const InputLayout &layout)
{
	for (auto i = 0u; i < vertex_buffers.size(); ++i)
	{
		auto index = state.u.drawCommand.num_vertex_buffers++;
		auto &dc = state.u.drawCommand;
		dc.vertex_buffers[index] = vertex_buffers[i].buffer;
		dc.vertex_buffers_offsets[index] = vertex_buffers[i].offset;
		dc.vertex_buffers_strides[index] = layout.strides[i];
	}
	setInputLayout(layout);
}

void RenderQueue::setIndexBuffer(const BufferDesc &index_buffer)
{
	state.u.drawCommand.index_buffer = index_buffer.buffer;
	state.u.drawCommand.index_buffer_offset = index_buffer.offset;
}

void RenderQueue::setUniformBuffers(util::array_ref<BufferDesc> uniform_buffers)
{
	for (auto i = 0u; i < uniform_buffers.size(); ++i)
	{
		auto index = state.u.drawCommand.num_uniform_buffers++;
		state.u.drawCommand.uniform_buffers[index] = uniform_buffers[i].buffer;
		state.u.drawCommand.uniform_buffers_offsets[index] = uniform_buffers[i].offset;
		state.u.drawCommand.uniform_buffers_sizes[index] = uniform_buffers[i].size;
	}
}

void RenderQueue::setTexture2D(int unit, const Texture2D &tex, const SamplerDesc &samplerDesc)
{
	auto index = state.u.drawCommand.num_textures++;
	state.u.drawCommand.textures[index] = tex.id;
	state.u.drawCommand.samplers[index] = Renderer::getInstance().getSampler(samplerDesc);
}

void RenderQueue::setTextureCubeMap(int unit, const TextureCubeMap &tex, const SamplerDesc &samplerDesc)
{
	auto index = state.u.drawCommand.num_textures++;
	state.u.drawCommand.textures[index] = tex.id;
	state.u.drawCommand.samplers[index] = Renderer::getInstance().getSampler(samplerDesc);
}

void RenderQueue::setShader(const Shader &shader)
{
	// TODO rename to pipeline state
	state.u.drawCommand.shader = &shader;
}

void RenderQueue::draw(
	PrimitiveType primitiveType,
	unsigned firstVertex,
	unsigned vertexCount,
	unsigned firstInstance,
	unsigned instanceCount)
{
	// TODO clean this (be type-safe)
	// => flexible command buffer
	state.u.drawCommand.first_vertex = firstVertex;
	state.u.drawCommand.vertex_count = vertexCount;
	state.u.drawCommand.index_count = 0;
	state.u.drawCommand.first_instance = firstInstance;
	state.u.drawCommand.instance_count = instanceCount;
	state.u.drawCommand.mode = detail::primitiveTypeToGLenum(primitiveType);
	state.type = RenderItem2::Type::DrawCommand;
	// end command
	// TODO no-copy
	render_items.push_back(state);
}

void RenderQueue::drawIndexed(
	PrimitiveType primitiveType,
	unsigned firstIndex,
	unsigned indexCount,
	int vertexOffset,
	unsigned firstInstance,
	unsigned instanceCount)
{
	state.u.drawCommand.first_index = firstIndex;
	state.u.drawCommand.first_vertex = vertexOffset;
	state.u.drawCommand.index_count = indexCount;
	state.u.drawCommand.vertex_count = 0;
	state.u.drawCommand.first_instance = firstInstance;
	state.u.drawCommand.instance_count = instanceCount;
	state.u.drawCommand.mode = detail::primitiveTypeToGLenum(primitiveType);
	state.type = RenderItem2::Type::DrawCommand;
	render_items.push_back(state);
}

void RenderQueue::fenceSync(GLsync &out_sync)
{
	state.u.fence.sync = &out_sync;
	state.type = RenderItem2::Type::FenceSync;
	render_items.push_back(state);
}

void RenderQueue::setInputLayout(const InputLayout &layout)
{
	state.u.drawCommand.input_layout = &layout;
}

void Renderer::commit(
	RenderQueue &renderQueue2
	)
{
	for (const auto &ri : renderQueue2.render_items) {
		drawItem2(ri);
	}
}

void Renderer::drawItem2(const RenderItem2 &item)
{

	if (item.type == RenderItem2::Type::DrawCommand)
	{
		auto &dc = item.u.drawCommand;
		if (item.u.drawCommand.input_layout != nullptr)
		{
			gl::BindVertexArray(dc.input_layout->vao);
			gl::BindVertexBuffers(0,
				dc.num_vertex_buffers,
				dc.vertex_buffers,
				dc.vertex_buffers_offsets,
				dc.vertex_buffers_strides);
		}
		else {
			// fully procedural (no VBO)
			gl::BindVertexArray(dummy_vao);
		}

		gl::BindBuffersRange(
			gl::UNIFORM_BUFFER,
			0,
			dc.num_uniform_buffers,
			dc.uniform_buffers,
			dc.uniform_buffers_offsets,
			dc.uniform_buffers_sizes
			);

		if (dc.num_textures)
		{
			gl::BindTextures(0, dc.num_textures, dc.textures);
			gl::BindSamplers(0, dc.num_textures, dc.samplers);
		}

		// shaders
		gl::UseProgram(dc.shader->program);
		// Rasterizer
		if (dc.shader->rs_state.cullMode == CullMode::None) {
			gl::Disable(gl::CULL_FACE);
		}
		else {
			gl::Enable(gl::CULL_FACE);
			gl::CullFace(detail::cullModeToGLenum(dc.shader->rs_state.cullMode));
		}
		gl::PolygonMode(gl::FRONT_AND_BACK, detail::fillModeToGLenum(dc.shader->rs_state.fillMode));
		gl::Enable(gl::DEPTH_TEST);
		if (!dc.shader->ds_state.depthTestEnable)
			gl::DepthFunc(gl::ALWAYS);
		else
			gl::DepthFunc(gl::LEQUAL);
		if (dc.shader->ds_state.depthWriteEnable)
			gl::DepthMask(gl::TRUE_);
		else
			gl::DepthMask(gl::FALSE_);

		// OM / blend state
		// XXX this ain't cheap
		// TODO blend state per color buffer
		gl::Enable(gl::BLEND);
		gl::BlendEquationSeparatei(
			0,
			detail::blendOpToGL(dc.shader->om_state.rgbOp),
			detail::blendOpToGL(dc.shader->om_state.alphaOp));
		gl::BlendFuncSeparatei(
			0,
			detail::blendFactorToGL(dc.shader->om_state.rgbSrcFactor),
			detail::blendFactorToGL(dc.shader->om_state.rgbDestFactor),
			detail::blendFactorToGL(dc.shader->om_state.alphaSrcFactor),
			detail::blendFactorToGL(dc.shader->om_state.alphaDestFactor));

		if (dc.index_count) {
			gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, dc.index_buffer);
			gl::DrawElementsInstancedBaseVertexBaseInstance(
				dc.mode,
				dc.index_count,
				gl::UNSIGNED_SHORT,
				reinterpret_cast<void*>(dc.index_buffer_offset + dc.first_index * 2),
				dc.instance_count,
				dc.first_vertex,
				dc.first_instance);
		}
		else {
			gl::DrawArraysInstancedBaseInstance(
				dc.mode,
				dc.first_vertex,
				dc.vertex_count,
				dc.instance_count,
				dc.first_instance);
		}
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
		gl::SamplerParameteri(id, gl::TEXTURE_MIN_FILTER, detail::textureFilterToGL(desc.minFilter));
		gl::SamplerParameteri(id, gl::TEXTURE_MAG_FILTER, detail::textureFilterToGL(desc.magFilter));
		gl::SamplerParameteri(id, gl::TEXTURE_WRAP_R, detail::textureAddressModeToGL(desc.addrU));
		gl::SamplerParameteri(id, gl::TEXTURE_WRAP_S, detail::textureAddressModeToGL(desc.addrV));
		gl::SamplerParameteri(id, gl::TEXTURE_WRAP_T, detail::textureAddressModeToGL(desc.addrW));
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
	detail::setDebugCallback();
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