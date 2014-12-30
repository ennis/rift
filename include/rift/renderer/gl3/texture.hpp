#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <globject.hpp>
#include <glm/glm.hpp>

class Texture : GLAPIObject
{
public:
	GL_IS_NULL_IMPL(id)

	~Texture()
	{
		if (id) {
			gl::DeleteTextures(1, &id);
		}
	}

	ElementFormat getPixelFormat() const
	{
		return pixelFormat;
	}

protected:

	void swap(Texture &&rhs)
	{
		std::swap(id, rhs.id);
		std::swap(bindingPoint, rhs.bindingPoint);
		std::swap(numMipLevels, rhs.numMipLevels);
		std::swap(pixelFormat, rhs.pixelFormat);
	}

	Texture() = default;

	Texture(GLuint id_, GLenum bindingPoint_, unsigned int numMipLevels_, ElementFormat pixelFormat_) :
		id(id_),
		bindingPoint(bindingPoint_),
		numMipLevels(numMipLevels_),
		pixelFormat(pixelFormat_)
	{}

	GLuint id = 0;
    GLenum bindingPoint;
    unsigned int numMipLevels;
    ElementFormat pixelFormat;
};

class Texture2D : public Texture
{
public:
	GL_MOVEABLE_OBJECT_IMPL(Texture2D)

	void swap(Texture2D &&rhs)
	{
		Texture::swap(std::move(rhs));
		std::swap(size, rhs.size);
	}

	const glm::ivec2 &getSize() const {
		return size;
	}

	void update(unsigned int mipLevel, glm::ivec2 offset, glm::ivec2 size, const void *data);


private:
	glm::ivec2 size;
};

class Texture1D : public Texture
{
public:
	GL_MOVEABLE_OBJECT_IMPL(Texture1D);

	void swap(Texture1D &&rhs)
	{
		Texture::swap(std::move(rhs));
		std::swap(size, rhs.size);
	}

	unsigned int getSize() const {
		return size;
	}

	void update(unsigned int mipLevel, int offset, unsigned int size, const void *data)
	{
		const auto &pf = getElementFormatInfoGL(pixelFormat);
		if (gl::exts::var_EXT_direct_state_access)
		{
			gl::TextureSubImage1DEXT(id, gl::TEXTURE_1D, mipLevel, offset, size, pf.externalFormat, pf.type, data);
		}
		else 
		{
			gl::BindTexture(bindingPoint, id);
			gl::TexSubImage1D(bindingPoint, mipLevel, offset, size, pf.externalFormat, pf.type, data);
			gl::BindTexture(bindingPoint, 0);
		}
	}

private:
	unsigned int size;
};

class TextureCubeMap : public Texture
{
public:
	GL_MOVEABLE_OBJECT_IMPL(TextureCubeMap);

	void swap(TextureCubeMap &&rhs)
	{
		Texture::swap(std::move(rhs));
		std::swap(size, rhs.size);
	}

	void update(
		unsigned int face, 
		unsigned int mipLevel, 
		glm::ivec2 offset, 
		glm::ivec2 size, 
		const void *data)
	{
		const auto &pf = getElementFormatInfoGL(pixelFormat);
		if (gl::exts::var_EXT_direct_state_access)
		{
			gl::TextureSubImage2DEXT(
				id, 
				gl::TEXTURE_CUBE_MAP_POSITIVE_X+face, 
				mipLevel, 
				offset.x, 
				offset.y, 
				size.x, 
				size.y, 
				pf.externalFormat, 
				pf.type, 
				data);
		}
		else
		{
			gl::BindTexture(gl::TEXTURE_CUBE_MAP, id);
			gl::TexSubImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_X+face, 
				mipLevel, 
				offset.x, 
				offset.y, 
				size.x, 
				size.y, 
				pf.externalFormat, 
				pf.type, 
				data);
			gl::BindTexture(gl::TEXTURE_CUBE_MAP, 0);
		}
	}

private:
	glm::ivec2 size;
};
 
#endif /* end of include guard: TEXTURE_HPP */