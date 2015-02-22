#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <renderer_common.hpp>
#include <glm/glm.hpp>

template <typename Backend>
class Texture2DBase : 
	public RendererObject<typename Backend::Texture2DImpl>
{
public:
	// nullable
	Texture2DBase() = default;
	// ctor
	Texture2DBase(Impl impl_) : impl(impl_)
	{}
	// noncopyable
	Texture2DBase(const Texture2DBase<Backend> &) = delete;
	Texture2DBase<Backend> &operator=(const Texture2DBase<Backend> &) = delete;
	// moveable
	Texture2DBase(Texture2DBase<Backend> &&rhs) {
		*this = std::move(rhs);
	}
	Texture2DBase &operator=(Texture2DBase<Backend> &&rhs) {
		std::swap(impl, rhs.impl);
		return *this;
	}

	~Texture2DBase() {
		Backend::getInstance().deleteTexture2D(impl);
	}

	Texture2DBase(
		glm::ivec2 size,
		int numMipLevels,
		ElementFormat pixelFormat,
		const void *data) :
		impl(
			Backend::getInstance().createTexture2D(
				size, 
				numMipLevels,
				pixelFormat, 
				data)) 
	{
	}

	void update(
		int mipLevel,
		glm::ivec2 offset,
		glm::ivec2 size,
		const void *data
		)
	{
		// TODO
	}

private:
	Impl impl;
};

 
#endif /* end of include guard: TEXTURE_HPP */