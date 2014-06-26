#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <common.hpp>
#include <renderresource.hpp>
#include <resourcemanager.hpp>

enum class TextureType
{
	Texture1D,
	Texture2D,
	Texture3D,
	TextureCubeMap
};

enum class PixelFormat
{
	R8G8B8A8 = 0,
	R8G8B8,
	A8B8G8R8,
	B8G8R8,
	R8,
	R16,
	R32,
	R32F,
	Depth,
	MaxPixelFormat
};

const char *pixelFormatToString(PixelFormat pf);

struct TextureDesc
{
	glm::ivec3 size;
	TextureType textureType;
	PixelFormat format;
	int numMipMapLevels;
};

struct CTexture : public CRenderResource
{
	TextureDesc mDesc;

	virtual void update(glm::ivec3 const &coords, glm::ivec3 const &size, void *data) = 0;
};

typedef Handle<CTexture> CTextureRef;

#endif