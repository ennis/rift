#ifndef TEXTURELOADER_HPP
#define TEXTURELOADER_HPP

#include <texture.hpp>
#include <renderer.hpp>

CTexture2DRef loadTexture2DFromFile(std::string path);

CTextureCubeMapRef loadTextureCubeMapFromFile(
	std::string positiveX,
	std::string negativeX,
	std::string positiveY,
	std::string negativeY,
	std::string positiveZ,
	std::string negativeZ);

#endif