#ifndef RENDERTARGET_HPP
#define RENDERTARGET_HPP

#include <globject.hpp>
#include <texture.hpp>

class RenderTarget : public GLAPIObject
{
public:
	GL_MOVEABLE_OBJECT_IMPL(RenderTarget)
	GL_IS_NULL_IMPL(texture)

	void swap(RenderTarget &&rhs)
	{
		std::swap(texture, rhs.texture);
		std::swap(mipLevel, rhs.mipLevel);
		std::swap(layer, rhs.layer);
	}

protected:
	RenderTarget(const Texture *texture_, unsigned int mipLevel_, int layer_) :
		texture(texture_),
		mipLevel(mipLevel_),
		layer(layer_)
	{}

	const Texture *texture;
	unsigned int mipLevel;
	int layer;	// -1 if no face or texture is a cube map and is bound as a layered image
};

 
#endif /* end of include guard: RENDERTARGET_HPP */