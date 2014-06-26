#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <common.hpp>
#include <resource.hpp>

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

template <typename pixel_type>
class Image 
{
public:
	Image();
	Image(glm::ivec2 size, pixel_type *data);

	glm::ivec2 size() const;

	pixel_type *getPixels();
	pixel_type operator() (int x, int y) const;
	pixel_type operator() (glm::ivec2 coords) const;
	pixel_type &operator() (int x, int y);
	pixel_type &operator() (glm::ivec2 coords);

	void free();

	pixel_type sample(float x, float y) const;
	pixel_type sample(glm::vec2 coords) const;

private:
	pixel_type *mData;
	glm::ivec2 mSize;
};

template <typename pixel_type>
std::unique_ptr<Image<pixel_type> > loadImageFromFile(const char *path, ImageLoadParameters const &imageLoadParameters);

std::unique_ptr<glm::u8> loadImageDataFromFile(const char *path, glm::ivec2 &size, int &numComponents);

typedef Image<glm::u8vec3> Image_U8_RGB;
typedef Image<glm::u8vec2> Image_U8_RG;
typedef Image<glm::u8> Image_U8_R;
typedef Image<glm::vec3> Image_Float_RGB;
typedef Image<glm::vec3> Image_Float_RG;

#endif