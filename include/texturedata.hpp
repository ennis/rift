#ifndef TEXTUREDATA_HPP
#define TEXTUREDATA_HPP

#include <imageview.hpp>
#include <istream>
#include <memory>

//=============================================================================
// texture data
// TODO 3D textures, texture arrays, cubemap textures
class TextureData : public Resource 
{
public:
	TextureData();
	~TextureData();
	void loadFromFile(const char *filePath);
	void allocate(ElementFormat format, glm::ivec3 size, int numMips);
	ElementFormat format() const {
		return mFormat;
	}
	glm::ivec2 size(int mipLevel = 0) const {
		assert(mipLevel < mNumMipLevels);
		return mMipMaps[mipLevel][0].size;
	}
	int numFaces() const {
		return mNumFaces;
	}
	int numMipLevels() const {
		return mNumMipLevels;
	}

	// use imageView(mipLevel, face).data()

	/*void *data(int mipLevel = 0, int face = 0) {
		assert(mipLevel < mNumMipLevels);
		assert(face < mNumFaces);
		return mMipData.get() + mMipMaps[mipLevel][0].offset;
	}*/

	int dataSize(int mipLevel = 0, int face = 0) {
		assert(mipLevel < mNumMipLevels);
		assert(face < mNumFaces);
		return mMipMaps[mipLevel][face].bytes;
	}
	BaseImageView imageView(int mipLevel = 0, int face = 0) {
		assert(mipLevel < mNumMipLevels);
		assert(face < mNumFaces);
		auto &mip = mMipMaps[mipLevel][face];
		return BaseImageView(
			mMipData.get()->ptr + mip.offset, 
			mFormat, 
			mip.size, 
			mip.size.x);
	}
	// return unique_ptr
	Texture2D *convertToTexture2D(Renderer &renderer);
	//------ DDS ------
	void loadDDS(std::istream &streamIn);
private:
	ElementFormat mFormat;
	glm::ivec2 mMainSize;
	int mNumMipLevels;
	int mNumFaces;
	static const int kMaxMipLevels = 32;
	static const int kMaxFaces = 6;
	// TODO rename MipData: a member with the same name already exists
	struct MipData {
		int offset;
		int bytes;
		glm::ivec2 size;
	};
	MipData mMipMaps[kMaxMipLevels][kMaxFaces];
	int mDataSize;
	// necessary for stb_image support (which uses malloc)
	// Abstract base class needed because reasons (unique_ptr madness and polymorphic deleters)
	struct PtrWrap {
		PtrWrap(unsigned char *ptr_) : ptr(ptr_)
		{}
		virtual ~PtrWrap() {}
		unsigned char *ptr;
	};
	struct MallocPtrWrap : public PtrWrap {
		MallocPtrWrap(unsigned char *ptr_) : PtrWrap(ptr_) {}
		~MallocPtrWrap() { free(ptr); }
	};
	struct DefaultPtrWrap : public PtrWrap {
		DefaultPtrWrap(unsigned char *ptr_) : PtrWrap(ptr_) {}
		~DefaultPtrWrap() { LOG << "DELETE ";  delete[] ptr; }
	};
	std::unique_ptr<PtrWrap> mMipData;
};

#endif