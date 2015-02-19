#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include <globject.hpp>

namespace {
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
}

class Sampler : public GLAPIObject 
{
public:
	GL_IS_NULL_IMPL(id)
	GL_MOVEABLE_OBJECT_IMPL(Sampler)

	Sampler(SamplerDesc desc_) : desc(desc_)
	{
		gl::GenSamplers(1, &id);
		gl::SamplerParameteri(id, gl::TEXTURE_MIN_FILTER, textureFilterToGL(desc.minFilter));
		gl::SamplerParameteri(id, gl::TEXTURE_MAG_FILTER, textureFilterToGL(desc.magFilter));
		gl::SamplerParameteri(id, gl::TEXTURE_WRAP_R, textureAddressModeToGL(desc.addrU));
		gl::SamplerParameteri(id, gl::TEXTURE_WRAP_S, textureAddressModeToGL(desc.addrV));
		gl::SamplerParameteri(id, gl::TEXTURE_WRAP_T, textureAddressModeToGL(desc.addrW));
	}

private:
	void swap(Sampler &&rhs)
	{
		std::swap(id, rhs.id);
	}

	GLuint id;
	SamplerDesc desc;
};

 
#endif /* end of include guard: SAMPLER_HPP */