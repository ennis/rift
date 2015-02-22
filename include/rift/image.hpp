#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <imageview.hpp>	// BaseImageView, ImageView<T>
#include <istream>
#include <vector>	// vector
#include <log.hpp>
#include <gl4/renderer.hpp>

enum class ImageFileFormat
{
	Autodetect,
	DDS,
	PNG,
	JPEG,
	TGA
};

//=============================================================================
// texture data
// TODO 3D textures, texture arrays, cubemap textures
class Image  
{
public:
	struct Subimage {
		std::size_t offset;
		std::size_t numBytes;
		glm::ivec2 size;
	};

	static const int kMaxMipLevels = 32;
	static const int kMaxFaces = 6;

	// default ctor (empty image), can only be moved into
	Image();
	Image(
		ElementFormat format, 
		glm::ivec3 size, 
		unsigned int numMipLevels = 1,
		unsigned int numFaces = 1
		);

	// copy
	Image(Image const &rhs);
	// move
	Image(Image &&rhs);
	~Image();

	// move-assign
	Image &operator=(Image &&rhs);

	ElementFormat getFormat() const {
		return mFormat;
	}

	glm::ivec2 getSize(unsigned int mipLevel = 0) const
	{
		assert(mipLevel < mNumMipLevels);
		return getSubimage(mipLevel, 0).size;
	}

	unsigned int getNumFaces() const
	{
		return mNumFaces;
	}

	unsigned int getNumMipLevels() const
	{
		return mNumMipLevels;
	}

	std::size_t getDataSize(
		unsigned int mipLevel = 0,
		unsigned int face = 0
		)
	{
		assert(mipLevel < mNumMipLevels);
		assert(face < mNumFaces);
		return getSubimage(mipLevel, face).numBytes;
	}

	BaseImageView getImageView(
		unsigned int mipLevel = 0,
		unsigned int face = 0
		)
	{
		assert(mipLevel < mNumMipLevels);
		assert(face < mNumFaces);
		auto &mip = getSubimage(mipLevel, face);
		return BaseImageView(
			mData.data() + mip.offset, 
			mFormat, 
			mip.size, 
			mip.size.x);
	}
	
	Texture2D convertToTexture2D();

	static Image loadFromFile(
		const char *filePath, 
		ImageFileFormat format = ImageFileFormat::Autodetect
		);

	static Image loadFromStream(
		std::istream &streamIn, 
		ImageFileFormat format
		);

	void loadDDS(std::istream &streamIn);

private:
	Subimage &getSubimage(
		unsigned int mipLevel,
		unsigned int face)
	{
		return mSubimages[mipLevel*mNumFaces + face];
	}

	Subimage const &getSubimage(
		unsigned int mipLevel,
		unsigned int face) const
	{
		return mSubimages[mipLevel*mNumFaces + face];
	}

	void allocate(
		ElementFormat format, 
		glm::ivec3 size, 
		unsigned int numMipLevels = 1,
		unsigned int numFaces = 1
		);

	ElementFormat mFormat;
	unsigned int mNumMipLevels;
	unsigned int mNumFaces;
	std::vector<Subimage> mSubimages;
	std::vector<unsigned char> mData;
};

#endif