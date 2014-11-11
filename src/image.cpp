#include <image.hpp>
#include <string>
#include <fstream>
#define STBI_HEADER_FILE_ONLY
#include "utils/stb_image.cpp"

//=============================================================================
// Default constructor
Image::Image() : 
mFormat(ElementFormat::Max),
mMainSize(glm::ivec2(0, 0)),
mNumMipLevels(0),
mNumFaces(0),
mDataSize(0),
mMipData(nullptr)
{
}

//=============================================================================
Image::~Image()
{
}

//=============================================================================
void Image::allocate(ElementFormat format, glm::ivec3 size, int numMips)
{
	// TODO
}

//=============================================================================
Texture2D *Image::convertToTexture2D(Renderer &renderer)
{
	auto tex = renderer.createTexture2D(
		mMainSize, mNumMipLevels, mFormat, 0, nullptr);
	for (int iMip = 0; iMip < mNumMipLevels; iMip++) {
		renderer.updateTexture2D(
			tex, 
			iMip, 
			glm::ivec2(0, 0), 
			size(iMip), 
			dataSize(iMip), 
			imageView(iMip).data());
	}
	return tex;
}

//=============================================================================
void Image::loadFromFile(const char *filePath)
{
	// extract extension
	std::string sp(filePath);
	auto dot = sp.find_last_of(".");
	if (dot == std::string::npos) {
		WARNING << "Image::loadFromFile: no extension, trying DDS\n";
		std::ifstream fileIn(filePath, std::ios::in | std::ios::binary);
		assert(fileIn.is_open());
		loadDDS(fileIn);
	} else {
		auto ext = sp.substr(dot+1, sp.size()-(dot+1));
		if (ext == "dds") {
			std::ifstream fileIn(filePath, std::ios::in | std::ios::binary);
			assert(fileIn.is_open());
			loadDDS(fileIn);
		} else if (ext == "tga" || ext == "png" || ext == "jpg") {
			// use stb_image
			int width;
			int height;
			int comp;
			unsigned char *data = 
				stbi_load(filePath, &width, &height, &comp, 0);
			mMainSize.x = width;
			mMainSize.y = height;
			mMipData = std::unique_ptr<PtrWrap>(new MallocPtrWrap(data));
			if (comp == 4) mFormat = ElementFormat::Unorm8x4;
			else if (comp == 3) mFormat = ElementFormat::Unorm8x3;
			else if (comp == 2) mFormat = ElementFormat::Unorm8x2;
			else if (comp == 1) mFormat = ElementFormat::Unorm8;
			else assert(false); 
			mDataSize = width * height * comp;
			mMipMaps[0][0].offset = 0;
			mMipMaps[0][0].bytes = mDataSize;
			mMipMaps[0][0].size = mMainSize;
			mNumMipLevels = 1;
			mNumFaces = 1;
		}
	}
}

