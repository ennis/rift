#include <rendering/opengl4.hpp>
#include <log.hpp>

// operator== for samplerDesc
bool operator==(const SamplerDesc &lhs, const SamplerDesc &rhs)
{
	return (lhs.addrU == rhs.addrU)
		&& (lhs.addrV == rhs.addrV)
		&& (lhs.addrW == rhs.addrW)
		&& (lhs.minFilter == rhs.minFilter)
		&& (lhs.magFilter == rhs.magFilter);
}

Buffer::~Buffer() {
	gc.deleteBuffer(*this);
}

void bindVertexBuffers(
	util::array_ref<const Buffer*> vertexBuffers,
	const VAO &vao)
{
	auto nbufs = vertexBuffers.size();
	assert(nbufs <= kMaxVertexBufferBindings);
	GLuint vbufs[kMaxVertexBufferBindings];
	GLintptr offsets[kMaxVertexBufferBindings];
	GLsizei strides[kMaxVertexBufferBindings];
	for (auto i = 0u; i < nbufs; ++i)
	{
		vbufs[i] = vertexBuffers[i]->obj;
		offsets[i] = vertexBuffers[i]->offset;
		strides[i] = vao.strides[i];
	}

	gl::BindVertexArray(vao.obj);
	gl::BindVertexBuffers(0,
		nbufs,
		vbufs,
		offsets,
		strides);
}

void bindBuffersRangeHelper(unsigned first, util::array_ref<const Buffer*> buffers)
{
	auto nbufs = buffers.size();
	assert(nbufs <= kMaxUniformBufferBindings);
	GLuint bufs[kMaxUniformBufferBindings];
	GLintptr offsets[kMaxUniformBufferBindings];
	GLsizeiptr sizes[kMaxUniformBufferBindings];
	for (auto i = 0u; i < nbufs; ++i)
	{
		bufs[i] = buffers[i]->obj;
		offsets[i] = buffers[i]->offset;
		sizes[i] = buffers[i]->size;
	}

	gl::BindBuffersRange(gl::UNIFORM_BUFFER, 0, nbufs, bufs, offsets, sizes);
}


void drawIndexed(
	GLenum mode,
	const Buffer &ib,
	unsigned firstVertex,
	unsigned firstIndex,
	unsigned indexCount,
	unsigned firstInstance,
	unsigned instanceCount)
{
	gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, ib.obj);
	gl::DrawElementsInstancedBaseVertexBaseInstance(
		mode,
		indexCount,
		gl::UNSIGNED_SHORT,
		reinterpret_cast<void*>(ib.offset + firstIndex * 2),
		instanceCount, firstVertex, firstInstance);
}

void doFullScreenPass(
	GLenum mode,
	unsigned numVertices)
{
	
}

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

GLuint GraphicsContext::getSamplerLinearClamp()
{
	if (!samLinearClamp)
	{
		gl::GenSamplers(1, &samLinearClamp);
		gl::SamplerParameteri(samLinearClamp, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
		gl::SamplerParameteri(samLinearClamp, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
		gl::SamplerParameteri(samLinearClamp, gl::TEXTURE_WRAP_R, gl::CLAMP_TO_EDGE);
		gl::SamplerParameteri(samLinearClamp, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
		gl::SamplerParameteri(samLinearClamp, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);
	}
	return samLinearClamp;
}

GLuint GraphicsContext::getSamplerNearestClamp()
{
	if (!samNearestClamp)
	{
		gl::GenSamplers(1, &samNearestClamp);
		gl::SamplerParameteri(samNearestClamp, gl::TEXTURE_MIN_FILTER, gl::NEAREST);
		gl::SamplerParameteri(samNearestClamp, gl::TEXTURE_MAG_FILTER, gl::NEAREST);
		gl::SamplerParameteri(samNearestClamp, gl::TEXTURE_WRAP_R, gl::CLAMP_TO_EDGE);
		gl::SamplerParameteri(samNearestClamp, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
		gl::SamplerParameteri(samNearestClamp, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);
	}
	return samNearestClamp;
}

GLuint GraphicsContext::getSamplerLinearRepeat()
{
	if (!samLinearRepeat)
	{
		gl::GenSamplers(1, &samLinearRepeat);
		gl::SamplerParameteri(samLinearRepeat, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
		gl::SamplerParameteri(samLinearRepeat, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
		gl::SamplerParameteri(samLinearRepeat, gl::TEXTURE_WRAP_R, gl::REPEAT);
		gl::SamplerParameteri(samLinearRepeat, gl::TEXTURE_WRAP_S, gl::REPEAT);
		gl::SamplerParameteri(samLinearRepeat, gl::TEXTURE_WRAP_T, gl::REPEAT);
	}
	return samNearestRepeat;
}

GLuint GraphicsContext::getSamplerNearestRepeat()
{
	if (!samNearestRepeat)
	{
		gl::GenSamplers(1, &samNearestRepeat);
		gl::SamplerParameteri(samNearestRepeat, gl::TEXTURE_MIN_FILTER, gl::NEAREST);
		gl::SamplerParameteri(samNearestRepeat, gl::TEXTURE_MAG_FILTER, gl::NEAREST);
		gl::SamplerParameteri(samNearestRepeat, gl::TEXTURE_WRAP_R, gl::REPEAT);
		gl::SamplerParameteri(samNearestRepeat, gl::TEXTURE_WRAP_S, gl::REPEAT);
		gl::SamplerParameteri(samNearestRepeat, gl::TEXTURE_WRAP_T, gl::REPEAT);
	}
	return samNearestRepeat;
}

void GraphicsContext::initialize()
{
	gl::GenVertexArrays(1, &dummy_vao); 
	setDebugCallback();
}

void GraphicsContext::tearDown()
{
	gl::DeleteVertexArrays(1, &dummy_vao);
	if (samLinearClamp)
		gl::DeleteSamplers(1, &samLinearClamp);
	if (samNearestClamp)
		gl::DeleteSamplers(1, &samNearestClamp);
	if (samLinearRepeat)
		gl::DeleteSamplers(1, &samLinearRepeat);
	if (samNearestRepeat)
		gl::DeleteSamplers(1, &samNearestRepeat);
}
	
void GraphicsContext::beginFrame()
{
	// reclaim transient buffers for frame n-2
	reclaimTransientBuffers();
}

void GraphicsContext::endFrame()
{
	syncTransientBuffers();
	// put fences
	frame_counter++;
}