#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <common.hpp>
#include <resource.hpp>
#include <renderer.hpp>

struct ImageLoadParameters
{
	int desiredComponents;
};

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

class BaseImageView
{
public:
	BaseImageView(void *data, ElementFormat format, glm::ivec2 size, int stride) : 
	mData(data), 
	mFormat(format), 
	mSize(size), 
	mStride(stride)
	{}

	template <typename pixel_type>
	pixel_type &at(float x, float y) const;

	glm::ivec2 size() const;

	void *getPixels();

protected:
	void *mData;
	ElementFormat mFormat;
	glm::ivec2 mSize;
	int mStride;
};

template <typename pixel_type>
class ImageView : public BaseImageView
{
public:
	ImageView(pixel_type *data, glm::ivec2 size, int stride) : 
	BaseImageView(data, ImageFormatTraits<pixel_type>::format, size, stride)
	{}


	pixel_type *getPixels();
	pixel_type operator() (int x, int y) const;
	pixel_type operator() (glm::ivec2 coords) const;
	pixel_type &operator() (int x, int y);
	pixel_type &operator() (glm::ivec2 coords);

	pixel_type sample(float x, float y) const;
	pixel_type sample(glm::vec2 coords) const;

private:
	pixel_type *mData;
	glm::ivec2 mSize;
};


#endif