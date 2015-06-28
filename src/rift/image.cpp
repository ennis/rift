#include <image.hpp>
#include <string>
#include <fstream>
#define STBI_HEADER_FILE_ONLY
#include "utils/stb_image.cpp"
#include <cmath>

namespace
{
	// https://code.google.com/p/imageresampler/
	double sinc(double x)
	{
		x = (x * glm::pi<double>());
		if ((x < 0.01) && (x > -0.01))
			return 1.0 + x*x*(-1.0 / 6.0 + x*x*1.0 / 120.0);
		return sin(x) / x;
	}

	double clean(double t)
	{
		constexpr double EPSILON = 0.0000125;
		if (fabs(t) < EPSILON)
			return 0.0;
		return t;
	}

	static double lanczosFilter(double t, double size)
	{
		if (t < 0.0)
			t = -t;
		if (t < size)
			return clean(sinc(t) * sinc(t / size));
		else
			return 0.0;
	}
}

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
	data.resize(len);

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
void Image::generateMipMaps()
{
	// allocate space for the mipmaps
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

