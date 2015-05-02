// OpenGL4+ renderer implementation
#include <renderer_common.hpp>
#include <renderer.hpp>
#include <gl_common.hpp>
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

namespace gl4
{
	GLuint dummy_vao;

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

namespace Renderer
{
	Sampler::Ptr sampler_LinearClamp;
	Sampler::Ptr sampler_NearestClamp;
	Sampler::Ptr sampler_LinearRepeat;
	Sampler::Ptr sampler_NearestRepeat;

	Sampler *getSampler_LinearClamp()
	{
		if (!sampler_LinearClamp)
			sampler_LinearClamp = Sampler::create(
			TextureAddressMode::Clamp,
			TextureAddressMode::Clamp,
			TextureAddressMode::Clamp,
			TextureFilter::Linear,
			TextureFilter::Linear);
		return sampler_LinearClamp.get();
	}

	Sampler *getSampler_NearestClamp()
	{
		if (!sampler_NearestClamp)
			sampler_NearestClamp = Sampler::create(
			TextureAddressMode::Clamp,
			TextureAddressMode::Clamp,
			TextureAddressMode::Clamp,
			TextureFilter::Nearest,
			TextureFilter::Nearest);
		return sampler_NearestClamp.get();
	}

	Sampler *getSampler_LinearRepeat()
	{
		if (!sampler_LinearRepeat)
			sampler_LinearRepeat = Sampler::create(
			TextureAddressMode::Repeat,
			TextureAddressMode::Repeat,
			TextureAddressMode::Repeat,
			TextureFilter::Linear,
			TextureFilter::Linear);
		return sampler_LinearRepeat.get();
	}

	Sampler *getSampler_NearestRepeat()
	{
		if (!sampler_NearestRepeat)
			sampler_NearestRepeat = Sampler::create(
			TextureAddressMode::Repeat,
			TextureAddressMode::Repeat,
			TextureAddressMode::Repeat,
			TextureFilter::Nearest,
			TextureFilter::Nearest);
		return sampler_NearestRepeat.get();
	}

	void initialize()
	{
		gl::GenVertexArrays(1, &gl4::dummy_vao); 
		gl4::setDebugCallback();
	}

	void tearDown()
	{
		gl::DeleteVertexArrays(1, &gl4::dummy_vao);
	}
}
