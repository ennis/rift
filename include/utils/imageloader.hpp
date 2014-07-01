#ifndef IMAGELOADER_HPP
#define IMAGELOADER_HPP

#include <image.hpp>
#include <resourcemanager.hpp>

std::unique_ptr<uint8_t> loadImageDataFromFile(const char *path, glm::ivec2 &size, int &numComponents);

#endif