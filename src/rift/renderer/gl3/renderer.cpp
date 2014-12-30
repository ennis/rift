#include <renderer.hpp>
#include <shader.hpp>
#include <gl3error.hpp>
#include <gl_common.hpp>

#include <memory>
#include <vector>

//=============================================================================
static Renderer *GRenderer;

//=============================================================================
Buffer::Buffer(GLuint obj, std::size_t size, ResourceUsage usage) : 
mObj(obj), mSize(size), mUsage(usage)
{}

VertexBuffer::VertexBuffer(GLuint obj, int elementSize, int numVertices, ResourceUsage usage) :
Buffer(obj, elementSize * numVertices, usage), mElementSize(elementSize), mNumVertices(numVertices)
{}

IndexBuffer::IndexBuffer(GLuint obj, int elementSize, int numIndices, ResourceUsage usage):
Buffer(obj, elementSize * numIndices, usage), mIndexSize(elementSize), mNumIndices(numIndices)
{}

ConstantBuffer::ConstantBuffer(GLuint obj, std::size_t size, ResourceUsage usage) :
Buffer(obj, size, usage)
{}

Sampler::Sampler(GLuint obj) : mObj(obj)
{}

Shader::Shader(GLuint program) : mProgram(program)
{}

Shader::~Shader()
{
	if (mProgram != -1)
		glDeleteProgram(mProgram);
}

std::unique_ptr<uint8_t[]> Shader::getProgramBinary(int &binaryLength)
{
	int len, wlen;
	GLenum binfmt;
	glGetProgramiv(mProgram, GL_PROGRAM_BINARY_LENGTH, &len);
	std::unique_ptr<uint8_t[]> buf = std::unique_ptr<uint8_t[]>(new uint8_t[len]);
	glGetProgramBinary(mProgram, len, &wlen, &binfmt, buf.get());
	binaryLength = len;
	return std::move(buf);
}

Texture2D::Texture2D(GLuint obj, ElementFormat pixelFormat, int numMipLevels, glm::ivec2 size) :
Texture(obj, GL_TEXTURE_2D, pixelFormat, numMipLevels, glm::ivec3(size, 1))
{}

TextureCubeMap::TextureCubeMap(GLuint obj, ElementFormat pixelFormat, int numMipLevels, glm::ivec3 size) :
Texture(obj, GL_TEXTURE_CUBE_MAP, pixelFormat, numMipLevels, size)
{}

Texture::Texture(GLuint obj, GLenum bindingPoint, ElementFormat pixelFormat, int numMipLevels, glm::ivec3 size) :
mObj(obj), mBindingPoint(bindingPoint), mNumMipLevels(numMipLevels), mPixelFormat(pixelFormat), mSize(size)
{}

RenderTarget::RenderTarget(Texture2D *texture) : mTexture(texture)
{}

//=============================================================================
static GLenum primitiveTypeToGLenum(PrimitiveType type)
{
	switch (type)
	{
	case PrimitiveType::Line:
		return GL_LINES;
	case PrimitiveType::Point:
		return GL_POINTS;
		//case PrimitiveType::Quads:
		//	return GL_QUADS;
	case PrimitiveType::Triangle:
		return GL_TRIANGLES;
	case PrimitiveType::TriangleStrip:
		return GL_TRIANGLE_STRIP;
	default:
		break;
	}

	return GL_POINTS;
}

//=============================================================================
static GLuint createUniformBuffer(std::size_t size)
{
	GLuint obj;
	GLCHECK(glGenBuffers(1, &obj));
	GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, obj));
	GLCHECK(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW));
	GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	return obj;
}

//=============================================================================
Texture2D *Renderer::createTexture2D(
	glm::ivec2 size,
	int numMipLevels,
	ElementFormat pixelFormat,
	int nBytes,
	const void *pixels)
{
	const auto &pf = getElementFormatInfoGL(pixelFormat);
	assert(numMipLevels > 0);
	GLuint obj;
	GLCHECK(glGenTextures(1, &obj));
	GLCHECK(glBindTexture(GL_TEXTURE_2D, obj));
	GLCHECK(glTexStorage2D(GL_TEXTURE_2D,
		numMipLevels,
		pf.internalFormat,
		size.x,
		size.y));
	// default sampling parameters
	GLCHECK(glTexParameteri(
		GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
	GLCHECK(glTexParameteri(
		GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// mipmaps?
	//GLCHECK(glGenerateMipmap(GL_TEXTURE_2D));
	GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
	Texture2D* tex = new Texture2D(obj, pixelFormat, numMipLevels, size);
	if (pixels)
		updateTexture2D(tex, 0, glm::ivec2(0, 0), size, nBytes, pixels);
	return tex;
}

//=============================================================================
RenderTarget *Renderer::createRenderTarget(Texture2D *texture)
{
	// dummy
	return new RenderTarget(texture);
}

//=============================================================================
GLenum resourceUsageToGLUsage(ResourceUsage usage)
{
	switch (usage) {
	case ResourceUsage::Dynamic:
		return GL_DYNAMIC_DRAW;
	case ResourceUsage::Static:
		return GL_STATIC_DRAW;
	default:
		return GL_STATIC_DRAW;
	}
}

//=============================================================================
void Renderer::updateTexture2D(
	Texture2D *texture2D, 
	int mipLevel, 
	glm::ivec2 offset, 
	glm::ivec2 size, 
	int nBytes, 
	const void *data)
{
	assert((offset.x + size.x) <= texture2D->mSize.x 
		&& (offset.y + size.y) <= texture2D->mSize.y);

	GLCHECK(glBindTexture(GL_TEXTURE_2D, texture2D->mObj));

	const auto &pf = getElementFormatInfoGL(texture2D->mPixelFormat);

	if (pf.compressed) {
		GLCHECK(glCompressedTexSubImage2D(
			GL_TEXTURE_2D,
			mipLevel,
			offset.x,
			offset.y,
			size.x,
			size.y,
			pf.internalFormat,
			nBytes,
			data));
	}
	else {
		GLCHECK(glTexSubImage2D(
			GL_TEXTURE_2D,
			mipLevel,
			offset.x,
			offset.y,
			size.x,
			size.y,
			pf.externalFormat,
			pf.type,
			data));
	}

	GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

//=============================================================================
static GLuint createBuffer(
	std::size_t size, 
	GLenum usage, 
	GLenum bindingPoint, 
	const void *initialData)
{
	GLuint obj;
	GLCHECK(glGenBuffers(1, &obj));
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, obj));
	GLCHECK(glBufferData(GL_ARRAY_BUFFER, size, initialData, usage));
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	return obj;
}

//=============================================================================
VertexBuffer *Renderer::createVertexBuffer(
	int elemSize, 
	int numVertices, 
	ResourceUsage resourceUsage, 
	const void *initialData)
{
	return new VertexBuffer(
		createBuffer(
			elemSize * numVertices, 
			resourceUsageToGLUsage(resourceUsage), 
			GL_ARRAY_BUFFER,
			initialData), 
		elemSize, numVertices, resourceUsage);
}

//=============================================================================
IndexBuffer *Renderer::createIndexBuffer(
	int indexSize, 
	int numIndices, 
	ResourceUsage resourceUsage, 
	const void *initialData)
{
	return new IndexBuffer(
		createBuffer(
			indexSize * numIndices, 
			resourceUsageToGLUsage(resourceUsage), 
			GL_ELEMENT_ARRAY_BUFFER, 
			initialData),
		indexSize, numIndices, resourceUsage);
}

//=============================================================================
ConstantBuffer *Renderer::createConstantBuffer(
	int size, 
	ResourceUsage usage, 
	const void *initialData)
{
	return new ConstantBuffer(
		createBuffer(
			size, 
			resourceUsageToGLUsage(usage), 
			GL_UNIFORM_BUFFER, 
			initialData),
		size, usage);
}

//=============================================================================
void Renderer::updateBuffer(
	Buffer *buffer,
	std::size_t offset, 
	std::size_t size, 
	const void *data)
{
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer->mObj));
	GLCHECK(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

//=============================================================================
VertexLayout *Renderer::createVertexLayout(
	int numElements, 
	const VertexElement *vertexElements)
{
	assert(numElements < VertexLayout::kNumVertexElements);
	VertexLayout *layout = new VertexLayout();
	layout->mNumElements = numElements;
	for (int i = 0; i < numElements; ++i) {
		layout->mVertexElements[i] = vertexElements[i];
	}
	return layout;
}

//=============================================================================
VertexLayout *Renderer::createVertexLayout2(
	int numElements, 
	const VertexElement2 *vertexElements)
{
	assert(numElements < VertexLayout::kNumVertexElements);
	VertexLayout *layout = new VertexLayout();
	layout->mNumElements = numElements;
	unsigned int offset[kNumVertexBufferSlots] = {0};
	for (int i = 0; i < numElements; ++i) {
		auto &e = layout->mVertexElements[i];
		e.inputSlot = i;
		e.bufferSlot = vertexElements[i].bufferSlot;
		e.offset = offset[e.bufferSlot];
		e.format = vertexElements[i].format;
		offset[e.bufferSlot] += getElementFormatSize(vertexElements[i].format);
	}
	for (int i = 0; i < numElements; ++i) {
		auto &e = layout->mVertexElements[i];
		e.stride = offset[e.bufferSlot];
	}
	return layout;
}

//=============================================================================
Shader *Renderer::createShader(
	const char *vertexShaderSource, 
	const char *fragmentShaderSource)
{
	GLuint vs_obj = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
	GLuint fs_obj = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
	GLuint program_obj;
	GLCHECK(program_obj = glCreateProgram());
	GLCHECK(glAttachShader(program_obj, vs_obj));
	GLCHECK(glAttachShader(program_obj, fs_obj));
	linkProgram(program_obj);
	Shader *sh = new Shader(program_obj);
	glDetachShader(program_obj, vs_obj);
	glDetachShader(program_obj, fs_obj);
	glDeleteShader(vs_obj);
	glDeleteShader(fs_obj);
	return sh;
}

//=============================================================================
void Renderer::setNamedConstantFloat(const char *name, float value)
{
	assert(mCurrentShader);
	GLuint location = glGetUniformLocation(mCurrentShader->mProgram, name);
	glUniform1f(location, value);
}

//=============================================================================
void Renderer::setNamedConstantFloat2(const char *name, glm::vec2 values)
{
	assert(mCurrentShader);
	GLuint location = glGetUniformLocation(mCurrentShader->mProgram, name);
	glUniform2fv(location, 1, glm::value_ptr(values));
}

//=============================================================================
void Renderer::setNamedConstantFloat3(const char *name, glm::vec3 values)
{
	assert(mCurrentShader);
	GLuint location = glGetUniformLocation(mCurrentShader->mProgram, name);
	glUniform3fv(location, 1, glm::value_ptr(values));
}

//=============================================================================
void Renderer::setNamedConstantFloat4(const char *name, glm::vec4 values)
{
	assert(mCurrentShader);
	GLuint location = glGetUniformLocation(mCurrentShader->mProgram, name);
	glUniform4fv(location, 1, glm::value_ptr(values));
}

//=============================================================================
void Renderer::setNamedConstantInt2(const char *name, glm::ivec2 values)
{
	assert(mCurrentShader);
	GLuint location = glGetUniformLocation(mCurrentShader->mProgram, name);
	glUniform2iv(location, 1, glm::value_ptr(values));
}

//=============================================================================
void Renderer::setNamedConstantInt(const char *name, int value)
{
	assert(mCurrentShader);
	GLuint location = glGetUniformLocation(mCurrentShader->mProgram, name);
	glUniform1i(location, value);
}

//=============================================================================
void Renderer::setNamedConstantMatrix4(const char *name, glm::mat4 matrix)
{
	assert(mCurrentShader);
	GLuint location = glGetUniformLocation(mCurrentShader->mProgram, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

//=============================================================================
void Renderer::setTexture(int textureSlot, Texture *texture)
{
	GLCHECK(glActiveTexture(GL_TEXTURE0 + textureSlot));
	GLCHECK(glBindTexture(texture->mBindingPoint, texture->mObj));
}

//=============================================================================
void Renderer::setNamedConstantBuffer(
	const char *name, 
	ConstantBuffer *buffer)
{
	GLuint location = glGetUniformBlockIndex(mCurrentShader->mProgram, name);
	GLCHECK(glBindBufferBase(GL_UNIFORM_BUFFER, location, buffer->mObj));
}

//=============================================================================
void Renderer::setVertexBuffer(
	int vertexBufferSlot, 
	VertexBuffer *buffer)
{
	assert(vertexBufferSlot < kNumVertexBufferSlots);
	mCurrentVertexBuffers[vertexBufferSlot] = buffer;
}

//=============================================================================
void Renderer::setIndexBuffer(IndexBuffer *buffer)
{
	mCurrentIndexBuffer = buffer;
}

//=============================================================================
void Renderer::setShader(Shader *shader)
{
	GLCHECK(glUseProgram(shader->mProgram));
	mCurrentShader = shader;
}

//=============================================================================
void Renderer::setConstantBuffer(
	int constantBufferSlot, 
	ConstantBuffer *buffer)
{
	GLCHECK(glBindBufferRange(
		GL_UNIFORM_BUFFER, 
		constantBufferSlot, 
		buffer->mObj, 
		0, 
		buffer->mSize));
}

//=============================================================================
// TODO single function
void Renderer::setConstantBufferRange(
	int constantBufferSlot, 
	ConstantBuffer *buffer, 
	int offset, int size)
{
	GLCHECK(glBindBufferRange(
		GL_UNIFORM_BUFFER, 
		constantBufferSlot, 
		buffer->mObj, 
		offset, 
		size));
}

//=============================================================================
void Renderer::setupVertexArrays()
{
	assert(mCurrentVertexLayout);
	for (int i = 0; i < mCurrentVertexLayout->mNumElements; ++i) {
		VertexElement const &elem = mCurrentVertexLayout->mVertexElements[i];
		const auto &fmt = getElementFormatInfoGL(elem.format);
		int attrib_index = elem.inputSlot;

		if (mCurrentVertexBuffers[elem.bufferSlot]) {
			GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 
				mCurrentVertexBuffers[elem.bufferSlot]->mObj));
		}
		GLCHECK(glEnableVertexAttribArray(attrib_index));
		GLCHECK(glVertexAttribPointer(
			elem.inputSlot,
			fmt.size,
			fmt.type,
			fmt.normalize,
			elem.stride,
			reinterpret_cast<void*>(elem.offset)));
	}
	for (int i = mCurrentVertexLayout->mNumElements; i < VertexLayout::kNumVertexElements; ++i) {
		GLCHECK(glDisableVertexAttribArray(i));
	}
	if (mCurrentIndexBuffer) {
		GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 
			mCurrentIndexBuffer->mObj));
	}
}

//=============================================================================
void Renderer::draw(
	PrimitiveType primitiveType,
	int vertexOffset, int numVertices)
{
	GLenum mode = primitiveTypeToGLenum(primitiveType);
	setupVertexArrays();
	glDrawArrays(mode, vertexOffset, numVertices);
}

//=============================================================================
void Renderer::drawIndexed(
	PrimitiveType primitiveType,
	int vertexOffset, int numVertices,
	int indexOffset, int numIndices)
{
	GLenum mode = primitiveTypeToGLenum(primitiveType);
	assert(mCurrentIndexBuffer);
	setupVertexArrays();
	glDrawElementsBaseVertex(
		mode,
		numIndices,
		GL_UNSIGNED_SHORT,
		(void*)(indexOffset * 2),
		vertexOffset);
}

//=============================================================================
void Renderer::bindRenderTargets(
	int numRenderTargets, 
	RenderTarget **colorTargets,
	RenderTarget *depthStencilTarget)
{
	assert(numRenderTargets < kNumRenderTargetSlots);
	mNumRenderTargets = numRenderTargets;
	if (mNumRenderTargets == 0) {
		// No render targets means render to screen
		// FIXME change that?
		GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		//GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
		//GLCHECK(glDrawBuffers(1, &drawBuffer));
	} else {
		GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, mFBO));
		for (int i = 0; i < numRenderTargets; ++i) {
			// TODO be able to bind different mip levels or faces 
			// (for cubemap textures)
			GLCHECK(glFramebufferTexture(
				GL_FRAMEBUFFER, 
				GL_COLOR_ATTACHMENT0+i, 
				colorTargets[i]->mTexture->mObj, 
				0));
		}
		// TODO depth stencil
		GLCHECK(glFramebufferTexture(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			depthStencilTarget->mTexture->mObj,
			0));
		// check fb completeness
		GLenum err;
		GLCHECK(err = glCheckFramebufferStatus(GL_FRAMEBUFFER));
		assert(err == GL_FRAMEBUFFER_COMPLETE);
		// enable draw buffers
		static const GLenum drawBuffers[kNumRenderTargetSlots] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT0+1,
			GL_COLOR_ATTACHMENT0+2,
			GL_COLOR_ATTACHMENT0+3,
			GL_COLOR_ATTACHMENT0+4,
			GL_COLOR_ATTACHMENT0+5,
			GL_COLOR_ATTACHMENT0+6,
			GL_COLOR_ATTACHMENT0+7
		};
		GLCHECK(glDrawBuffers(mNumRenderTargets, drawBuffers));
	}
}

//=============================================================================
void Renderer::setViewports(
	int numViewports,
	const Viewport *viewports)
{
	static const int kMaxViewports = 32;
	assert(numViewports <= kMaxViewports);
	for (int i = 0; i < numViewports; ++i) {
		float vp[4] = {
			viewports[i].topLeftX,
			viewports[i].topLeftY, 
			viewports[i].width, 
			viewports[i].height
		};
		GLCHECK(glViewportIndexedfv(i, vp));
	}
}

//=============================================================================
void Renderer::setVertexLayout(VertexLayout *vertexLayout)
{
	mCurrentVertexLayout = vertexLayout;
}

//=============================================================================
void Renderer::initialize()
{
	// état par défaut du pipeline
	GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	// 226.f / 255.f, 236.f / 255.f, 124.f / 255.f, 1.f
	GLCHECK(glClearColor(
		mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a));
	GLCHECK(glClearDepth(mClearDepth));
	GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	GLCHECK(glEnable(GL_DEPTH_TEST));
	GLCHECK(glDisable(GL_CULL_FACE));
	// create framebuffer
	glGenFramebuffers(1, &mFBO);
}

//=============================================================================
void Renderer::clearColor(glm::vec4 const &color)
{
	GLCHECK(glClearColor(color.r, color.g, color.b, color.a));
	GLCHECK(glClear(GL_COLOR_BUFFER_BIT));
}

//=============================================================================
void Renderer::clearDepth(float depth)
{
	GLCHECK(glClearDepth(depth));
	GLCHECK(glClear(GL_DEPTH_BUFFER_BIT));
}

//=============================================================================
void Renderer::setRenderStates(
	CullMode cullMode, 
	PolygonFillMode polyFillMode,
	bool depthTestEnable,
	bool depthWriteEnable)
{
	// TODO implement
}

