#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <gl_common.hpp>
#include <glm/glm.hpp>
#include <utility>	// move

class Texture 
{
public:
	friend class Renderer;
	Texture() = default;
	Texture(const Texture&) = delete;
	Texture &operator=(const Texture&) = delete;
	Texture(Texture &&rhs)
	{
		*this = std::move(rhs);
	}

	Texture &operator=(Texture &&rhs)
	{
		id = rhs.id;
		bindingPoint = rhs.bindingPoint;
		numMipLevels = rhs.numMipLevels;
		pixelFormat = rhs.pixelFormat;
		rhs.id = 0;
		return *this;
	}

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

	bool isNull() const {
		return id == 0;
	}

protected:

	Texture(GLenum bindingPoint_, unsigned int numMipLevels_, ElementFormat pixelFormat_) :
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
	friend class Renderer;
	Texture2D() = default;
	Texture2D(const Texture2D&) = delete;
	Texture2D &operator=(const Texture2D&) = delete;
	Texture2D(Texture2D &&rhs)
	{
		*this = std::move(rhs);
	}

	Texture2D &operator=(Texture2D &&rhs) {
		size = rhs.size;
		Texture::operator=(std::move(rhs));
		return *this;
	}

	Texture2D(
		glm::ivec2 size_,
		unsigned int numMipLevels_,
		ElementFormat pixelFormat_,
		const void *data = nullptr
		);

	const glm::ivec2 &getSize() const {
		return size;
	}

	void update(
		unsigned int mipLevel,
		glm::ivec2 offset, 
		glm::ivec2 size, 
		const void *data
		);

private:
	glm::ivec2 size;
};

class Texture1D : public Texture
{
public:
	friend class Renderer;
	Texture1D() = delete;
	Texture1D(const Texture1D&) = delete;
	Texture1D &operator=(const Texture1D&) = delete;
	Texture1D(Texture1D &&rhs) 
	{
		*this = std::move(rhs);
	}
	Texture1D &operator=(Texture1D &&rhs) 
	{
		size = rhs.size;
		Texture::operator=(std::move(rhs));
		return *this;
	}

	Texture1D(
		unsigned int size_,
		unsigned int numMipLevels_,
		ElementFormat pixelFormat_
		);

	int getSize() const {
		return size;
	}

	void update(
		unsigned int mipLevel,
		unsigned int offset,
		unsigned int size,
		const void *data
		);

private:
	unsigned int size;
};

class TextureCubeMap : public Texture
{
public:
	friend class Renderer;
	TextureCubeMap() = delete;
	TextureCubeMap(const TextureCubeMap&) = delete;
	TextureCubeMap &operator=(const TextureCubeMap&) = delete;
	TextureCubeMap(TextureCubeMap &&rhs)
	{
		*this = std::move(rhs);
	}
	TextureCubeMap &operator=(TextureCubeMap &&rhs)
	{
		size = rhs.size;
		Texture::operator=(std::move(rhs));
		return *this;
	}

	TextureCubeMap(
		glm::ivec2 size_,
		unsigned int numMipLevels_,
		ElementFormat pixelFormat_,
		const void* faceData[6]
		);

	void update(
		unsigned int face,
		unsigned int mipLevel,
		glm::ivec2 offset,
		glm::ivec2 size,
		const void *data
		);

private:
	glm::ivec2 size;
};
 
#endif /* end of include guard: TEXTURE_HPP */