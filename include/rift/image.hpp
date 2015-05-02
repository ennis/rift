#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <imageview.hpp>	// BaseImageView, ImageView<T>
#include <istream>
#include <vector>	// vector
#include <small_vector.hpp>
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
		ElementFormat format_, 
		glm::ivec3 size_, 
		unsigned int numMipLevels_ = 1,
		unsigned int numFaces_ = 1
		);

	// copy
	Image(Image const &rhs);
	// move
	Image(Image &&rhs);
	~Image();

	// move-assign
	Image &operator=(Image &&rhs);

	ElementFormat getFormat() const {
		return format;
	}

	glm::ivec2 getSize(unsigned int mipLevel = 0) const
	{
		assert(mipLevel < numMipLevels);
		return getSubimage(mipLevel, 0).size;
	}

	unsigned int getNumFaces() const
	{
		return numFaces;
	}

	unsigned int getNumMipLevels() const
	{
		return numMipLevels;
	}

	std::size_t getDataSize(
		unsigned int mipLevel = 0,
		unsigned int face = 0
		)
	{
		assert(mipLevel < numMipLevels);
		assert(face < numFaces);
		return getSubimage(mipLevel, face).numBytes;
	}

	BaseImageView getImageView(
		unsigned int mipLevel = 0,
		unsigned int face = 0
		)
	{
		assert(mipLevel < numMipLevels);
		assert(face < numFaces);
		auto &mip = getSubimage(mipLevel, face);
		return BaseImageView(
			data.data() + mip.offset,
			format,
			mip.size, 
			mip.size.x);
	}
	
	Texture2D::Ptr convertToTexture2D();
	TextureCubeMap::Ptr convertToTextureCubeMap();

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
		return subimages[mipLevel][face];
	}

	Subimage const &getSubimage(
		unsigned int mipLevel,
		unsigned int face) const
	{
		return subimages[mipLevel][face];
	}

	void allocate(
		ElementFormat format_, 
		glm::ivec3 size_, 
		unsigned int numMipLevels_ = 1,
		unsigned int numFaces_ = 1
		);

	ElementFormat format;
	unsigned int numMipLevels;
	unsigned int numFaces;
	// TODO smallvector
	std::vector<util::small_vector<Subimage, 6> > subimages;
	std::vector<unsigned char> data;
};

#endif