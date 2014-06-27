#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <common.hpp>
#include <resourcemanager.hpp>
#include <renderresource.hpp>

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
	PixelFormat format;
	int numMipMapLevels;
};

struct Texture2DDesc : public TextureDesc
{
	glm::ivec2 size;
};

struct TextureCubeMapDesc : public TextureDesc
{
	glm::ivec2 size;
};


struct CTexture2D : public CRenderResource
{
	Texture2DDesc mDesc;

	virtual void update(int mipLevel, glm::ivec2 coords, glm::ivec2 size, const void *data) = 0;
};

enum CubeMapFace
{
	CubeMap_PositiveX = 0,
	CubeMap_NegativeX,
	CubeMap_PositiveY,
	CubeMap_NegativeY,
	CubeMap_PositiveZ,
	CubeMap_NegativeZ,
};

struct CTextureCubeMap : public CRenderResource
{
	TextureCubeMapDesc mDesc;

	virtual void update(int mipLevel, int face, glm::ivec2 coords, glm::ivec2 size, const void *data) = 0;
};


typedef Handle<CTexture2D> CTexture2DRef;
typedef Handle<CTextureCubeMap> CTextureCubeMapRef;

#endif