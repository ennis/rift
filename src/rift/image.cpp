#include <image.hpp>
#include <string>
#include <fstream>
#define STBI_HEADER_FILE_ONLY
#include "utils/stb_image.cpp"

//====================================
// Default constructor
Image::Image() : 
	mFormat(ElementFormat::Max),
	mNumMipLevels(0),
	mNumFaces(0)
{
}

//====================================
Image::Image(ElementFormat format, glm::ivec3 size, unsigned int numMipLevels, unsigned int numFaces)
{
	allocate(format, size, numMipLevels, numFaces);
}

//====================================
void Image::allocate(ElementFormat format, glm::ivec3 size, unsigned int numMipLevels, unsigned int numFaces)
{
	mFormat = format;
	mNumMipLevels = numMipLevels;
	mNumFaces = numFaces;

	auto elemsize = getElementFormatSize(format);
	std::size_t len = size.x * size.y * size.z * elemsize * numFaces * numMipLevels;
	std::size_t offset = 0;
	glm::ivec2 mipsize = glm::ivec2(size.x, size.y);
	mData.reserve(len);

	for (unsigned int iMip = 0; iMip < numMipLevels && mipsize.x != 1 && mipsize.y != 1; ++iMip)
	{
		for (unsigned int iFace = 0; iFace < numFaces; ++iFace)
		{
			std::size_t numBytes = mipsize.x * mipsize.y * elemsize;
			mSubimages.push_back(Subimage{ offset, numBytes, mipsize });
			offset += numBytes;
		}
		mipsize /= 2;
	}
}

//====================================
Image::Image(Image const &rhs) :
	mFormat(rhs.mFormat),
	mNumMipLevels(rhs.mNumMipLevels),
	mNumFaces(rhs.mNumFaces),
	mSubimages(rhs.mSubimages),
	mData(rhs.mData)
{
}

//====================================
Image::Image(Image &&rhs)
{
	*this = std::move(rhs);
}

//====================================
Image &Image::operator=(Image &&rhs)
{
	mFormat = rhs.mFormat;
	mNumMipLevels = rhs.mNumMipLevels;
	mNumFaces = rhs.mNumFaces;
	mData.swap(rhs.mData);
	mSubimages.swap(rhs.mSubimages);
	return *this;
}

//====================================
Image::~Image()
{
}

//====================================
Texture2D Image::convertToTexture2D(Renderer &renderer)
{
	auto tex = Texture2D(getSize(), mNumMipLevels, mFormat, nullptr);
	for (unsigned int iMip = 0; iMip < mNumMipLevels; iMip++) {
		tex.update(
			iMip, 
			glm::ivec2(0, 0), 
			getSize(iMip),
			getImageView(iMip).data());
	}
	return std::move(tex);
}

//====================================
namespace
{
	// fill 'data' with 'size' bytes.  return number of bytes actually read 
	int imageStbiRead(void *user, char *data, int size) 
	{
		std::istream *is = static_cast<std::istream*>(user);
		is->read(data, size);
		return static_cast<int>(is->gcount());
	}

	// skip the next 'n' bytes
	void imageStbiSkip(void *user, unsigned n)
	{
		std::istream *is = static_cast<std::istream*>(user);
		is->ignore(n);
	}

	// returns nonzero if we are at end of file/data
	int imageStbiEof(void *user)
	{
		std::istream *is = static_cast<std::istream*>(user);
		return is->eof() ? 1 : 0;
	}
}

//====================================
Image Image::loadFromStream(std::istream &streamIn, ImageFileFormat format)
{
	if (format == ImageFileFormat::DDS) {
		Image img = Image();
		img.loadDDS(streamIn);
		return img;
	}
	else {
		stbi_io_callbacks callbacks;
		callbacks.read = imageStbiRead;
		callbacks.skip = imageStbiSkip;
		callbacks.eof = imageStbiEof;
		int width;
		int height;
		int comp;
		unsigned char *data = stbi_load_from_callbacks(&callbacks, &streamIn, &width, &height, &comp, 0);
		auto dataSize = width * height * comp;
		ElementFormat fmt;
		if (comp == 4) fmt = ElementFormat::Unorm8x4;
		else if (comp == 3) fmt = ElementFormat::Unorm8x3;
		else if (comp == 2) fmt = ElementFormat::Unorm8x2;
		else if (comp == 1) fmt = ElementFormat::Unorm8;
		else assert(false);
		// allocate a new image
		Image img = Image(fmt, glm::ivec3(width, height, 1));
		// copy data into the new image
		std::memcpy(img.getImageView().data(), data, dataSize);
		return img;
	}
}

//====================================
Image Image::loadFromFile(const char *filePath, ImageFileFormat format)
{
	// extract extension
	std::string sp(filePath);
	auto dot = sp.find_last_of(".");
	if (dot == std::string::npos) {
		WARNING << "Image::loadFromFile: no extension, trying DDS\n";
		std::ifstream fileIn(filePath, std::ios::in | std::ios::binary);
		assert(fileIn.is_open());
		Image img = Image();
		img.loadDDS(fileIn);
		return img;
	} else {
		auto ext = sp.substr(dot+1, sp.size()-(dot+1));
		if (ext == "dds") {
			std::ifstream fileIn(filePath, std::ios::in | std::ios::binary);
			assert(fileIn.is_open());
			Image img = Image();
			img.loadDDS(fileIn);
			return img;
		} else {
			// use stbimage anyway
			std::ifstream fileIn(filePath, std::ios::in | std::ios::binary);
			assert(fileIn.is_open());			// use stb_image
			return loadFromStream(fileIn, ImageFileFormat::Autodetect);
		}
	}
}

