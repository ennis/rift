#include <image.hpp>
#include <string>
#include <fstream>
#define STBI_HEADER_FILE_ONLY
#include "utils/stb_image.cpp"

//====================================
// Default constructor
Image::Image() : 
	format(ElementFormat::Max),
	numMipLevels(0),
	numFaces(0)
{
}

//====================================
Image::Image(
	ElementFormat format_, 
	glm::ivec3 size_, 
	unsigned int numMipLevels_, 
	unsigned int numFaces_
	)
{
	allocate(format_, size_, numMipLevels_, numFaces_);
}

//====================================
void Image::allocate(
	ElementFormat format_, 
	glm::ivec3 size_, 
	unsigned int numMipLevels_,
	unsigned int numFaces_
	)
{
	format = format_;
	numMipLevels = numMipLevels_;
	numFaces = numFaces_;

	auto elemsize = getElementFormatSize(format);
	std::size_t len = size_.x * size_.y * size_.z * elemsize * numFaces * numMipLevels;
	std::size_t offset = 0;
	glm::ivec2 mipsize = glm::ivec2(size_.x, size_.y);
	data.reserve(len);

	for (auto imip = 0u; 
		(imip < numMipLevels) && (mipsize.x != 1) && (mipsize.y != 1); 
		++imip)
	{
		subimages.push_back(util::small_vector<Subimage, 6>());
		for (auto iface = 0u; iface < numFaces; ++iface)
		{
			std::size_t numBytes = mipsize.x * mipsize.y * elemsize;
			subimages[imip].push_back(Subimage{ offset, numBytes, mipsize });
			offset += numBytes;
		}
		mipsize /= 2;
	}
}

//====================================
Image::Image(Image const &rhs) :
	format(rhs.format),
	numMipLevels(rhs.numMipLevels),
	numFaces(rhs.numFaces),
	subimages(rhs.subimages),
	data(rhs.data)
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
	format = rhs.format;
	numMipLevels = rhs.numMipLevels;
	numFaces = rhs.numFaces;
	data.swap(rhs.data);
	subimages.swap(rhs.subimages);
	return *this;
}

//====================================
Image::~Image()
{
}

//====================================
Texture2D::Ptr Image::convertToTexture2D()
{
	auto tex = Texture2D::create(getSize(), numMipLevels, format, nullptr);
	for (auto imip = 0u; imip < numMipLevels; imip++) {
		tex->update(
			imip,
			glm::ivec2(0, 0), 
			getSize(imip),
			getImageView(imip).data());
	}
	return std::move(tex);
}

TextureCubeMap::Ptr Image::convertToTextureCubeMap()
{
	assert(numFaces == 6);
	const void *faceBytes[6] = {
		getImageView(0, 0).data(),
		getImageView(0, 1).data(),
		getImageView(0, 2).data(),
		getImageView(0, 3).data(),
		getImageView(0, 4).data(),
		getImageView(0, 5).data()
	};
	auto tex = TextureCubeMap::create(getSize(), numMipLevels, format, faceBytes);
	return tex;
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

