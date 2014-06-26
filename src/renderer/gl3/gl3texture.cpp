#include <gl3texture.hpp>
#include <gl3rendererimpl.hpp>
#include <gl3error.hpp>
#include <log.hpp>

static GLenum pixelFormatToGLenumInternalFormat(PixelFormat pf)
{
	switch (pf)
	{
	case PixelFormat::R8G8B8A8:
		return GL_RGBA8;
	case PixelFormat::R8G8B8:
		return GL_RGB8;
	case PixelFormat::A8B8G8R8:
		break;
	case PixelFormat::B8G8R8:
		break;
	case PixelFormat::R8:
		break;
	case PixelFormat::R16:
		break;
	case PixelFormat::R32:
		break;
	case PixelFormat::R32F:
		break;
	case PixelFormat::Depth:
		return GL_DEPTH_COMPONENT24;
	case PixelFormat::MaxPixelFormat:
		break;
	default:
		break;
	}
	return -1;
}

static GLenum pixelFormatToGLenumExternalFormat(PixelFormat pf)
{
	switch (pf)
	{
	case PixelFormat::R8G8B8A8:
		return GL_RGBA;
	case PixelFormat::R8G8B8:
		return GL_RGB;
	case PixelFormat::A8B8G8R8:
		break;
	case PixelFormat::B8G8R8:
		break;
	case PixelFormat::R8:
		break;
	case PixelFormat::R16:
		break;
	case PixelFormat::R32:
		break;
	case PixelFormat::R32F:
		break;
	case PixelFormat::Depth:
		break;
	case PixelFormat::MaxPixelFormat:
		break;
	default:
		break;
	}
	return GL_RGB;
}

static GLenum pixelFormatToGLenumComponent(PixelFormat pf)
{
	switch (pf)
	{
	case PixelFormat::R8G8B8A8:
		return GL_UNSIGNED_BYTE;
	case PixelFormat::R8G8B8:
		return GL_UNSIGNED_BYTE;
	case PixelFormat::A8B8G8R8:
		break;
	case PixelFormat::B8G8R8:
		break;
	case PixelFormat::R8:
		break;
	case PixelFormat::R16:
		break;
	case PixelFormat::R32:
		break;
	case PixelFormat::R32F:
		break;
	case PixelFormat::Depth:
		break;
	case PixelFormat::MaxPixelFormat:
		break;
	default:
		break;
	}
	return GL_UNSIGNED_BYTE;
}

CGL3Texture::CGL3Texture(TextureDesc &desc) : mObj(-1)
{
	mDesc = desc;
	initialize();
}

CGL3Texture::~CGL3Texture()
{
}

void CGL3Texture::initialize()
{
	GLCHECK(glGenTextures(1, &mObj));
	// TODO 1D, 2D, 3D textures, cubemaps, texture arrays, etc.
	GLCHECK(glBindTexture(GL_TEXTURE_2D, mObj));
	// allocate storage
	GLCHECK(glTexStorage2D(GL_TEXTURE_2D, 1, pixelFormatToGLenumInternalFormat(mDesc.format), mDesc.size.x, mDesc.size.y));
	// default sampling parameters
	GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
	GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// mipmaps?
	if (mDesc.numMipMapLevels > 0) {
		GLCHECK(glGenerateMipmap(GL_TEXTURE_2D));
	}
	GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

void CGL3Texture::deleteResource()
{
	GLCHECK(glDeleteTextures(1, &mObj));
	LOG << "CGL3Texture::deleteResource";
	// later, remove ourselves from the pool
	delete this;
}

void CGL3Texture::update(glm::ivec3 const &coords, glm::ivec3 const &size, void *pixels)
{
	assert(mObj != -1);
	GLCHECK(glBindTexture(GL_TEXTURE_2D, mObj));
	GLCHECK(glTexSubImage2D(GL_TEXTURE_2D, coords.z, coords.x, coords.y, size.x, size.y,
		pixelFormatToGLenumExternalFormat(mDesc.format), 
		pixelFormatToGLenumComponent(mDesc.format), 
		pixels));
	GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
}


void CGL3Texture::setActive(int textureUnit)
{
	assert(mObj != -1);
	GLCHECK(glActiveTexture(GL_TEXTURE0 + textureUnit));
	GLCHECK(glBindTexture(GL_TEXTURE_2D, mObj));
}
