#ifndef DDS_HPP
#define DDS_HPP
#include <renderer.hpp>
#include <resource.hpp>
#include <istream>

// DDS loader
// TODO return unique_ptr
Texture2D *loadTexture2D_DDS(Renderer &renderer, const char *ddsFilePath);
//Texture2D *loadTextureCubeMap_DDS(Renderer &renderer, const char *ddsFilePath);

#endif