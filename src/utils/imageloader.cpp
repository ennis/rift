#include <imageloader.hpp>
#include <log.hpp>

#define STBI_HEADER_FILE_ONLY
#include "stb_image.cpp"

std::unique_ptr<uint8_t> loadImageDataFromFile(const char *path, glm::ivec2 &size, int &numComponents)
{
	// load image using stb_image
	LOG << "loading " << path;
	unsigned char * data = stbi_load(path, &size.x, &size.y, &numComponents, 4);
	if (!data) {
		throw std::runtime_error("loadImageDataFromFile: stbi_load failed");
	}
	return std::unique_ptr<uint8_t>(data);
}
