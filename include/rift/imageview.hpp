#ifndef IMAGEVIEW_HPP
#define IMAGEVIEW_HPP

#include <common.hpp>	// glm
#include <renderer_common.hpp>	// ElementFormat

//=============================================================================
// Unused
enum class ImageWrappingMode
{
	Clamp,
	Repeat,
	Mirror
};

enum class ImageSamplingMode
{
	Nearest,
	Bilinear,
	Bicubic,
};

//=============================================================================
// Forward decls
template <typename pixel_type>
class ImageView;

//=============================================================================
// TODO structs and traits for each pixel type
// trait: Type -> format, normalized, integer, number of components, base type 
class BaseImageView
{
public:
	BaseImageView() : 
	mData(nullptr), 
	mFormat(ElementFormat::Max), 
	mSize(glm::ivec2(0, 0)), 
	mStride(0)
	{}

	BaseImageView(void *data, ElementFormat format, glm::ivec2 size, int stride) : 
	mData(data), 
	mFormat(format), 
	mSize(size), 
	mStride(stride)
	{}
	template <typename pixel_type>
	pixel_type &at(unsigned int x, unsigned int y)
	{
		// XXX not type-safe (verify size of pixel_type before)
		return *((pixel_type*)mData + y * mStride + x);
	}
	template <typename pixel_type>
	pixel_type at(unsigned int x, unsigned int y) const
	{
		// XXX not type-safe (verify size of pixel_type before)
		return *((const pixel_type*)mData + y * mStride + x);
	}
	glm::ivec2 size() const {
		return mSize;
	}
	void *data() {
		return mData;
	}
	int stride() const {
		return mStride;
	}
	ElementFormat format() const {
		return mFormat;
	}
	template <typename pixel_type>
	ImageView<pixel_type> viewAs();
protected:
	void *mData;
	ElementFormat mFormat;
	glm::ivec2 mSize;
	// stride in number of pixels
	unsigned int mStride;
};

//=============================================================================
// Same as the above, with fixed pixel type
template <typename pixel_type>
class ImageView : public BaseImageView
{
public:
	ImageView() : BaseImageView()
	{}
	ImageView(void *data, ElementFormat format, glm::ivec2 size, unsigned int stride) :
	BaseImageView(data, format, size, stride)
	{}
	pixel_type &operator()(unsigned int x, unsigned int y) {
		return at<pixel_type>(x, y);
	}
	pixel_type operator()(unsigned int x, unsigned int y) const {
		return at<pixel_type>(x, y);
	}
};

//=============================================================================
// BaseImageView::viewAs
template <typename pixel_type>
ImageView<pixel_type> BaseImageView::viewAs()
{
	return ImageView<pixel_type>(mData, mFormat, mSize, mStride);
}

#endif
