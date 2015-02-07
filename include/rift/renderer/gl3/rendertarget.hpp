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

	// TODO query format
	// TODO special object for screen RT

	ElementFormat getPixelFormat() const 
	{
		return format;
	}


	static RenderTarget createRenderTarget2D(Texture2D *texture2D, unsigned int mipLevel) 
	{
		return RenderTarget(texture2D, mipLevel, -1);
	}

	static RenderTarget createRenderTarget2D(TextureCubeMap *cubeMap, unsigned int mipLevel, unsigned int face)
	{
		return RenderTarget(cubeMap, mipLevel, face);
	}

	static RenderTarget createRenderTargetCubeMap(TextureCubeMap *cubeMap, unsigned int mipLevel)
	{
		return RenderTarget(cubeMap, mipLevel, -1);
	}

protected:
	RenderTarget(const Texture *texture_, unsigned int mipLevel_, int layer_) :
		format(texture_->getPixelFormat()),
		texture(texture_),
		mipLevel(mipLevel_),
		layer(layer_)
	{}

	ElementFormat format;
	const Texture *texture;
	unsigned int mipLevel;
	int layer;	// -1 if no face or texture is a cube map and is bound as a layered image
};

 
#endif /* end of include guard: RENDERTARGET_HPP */