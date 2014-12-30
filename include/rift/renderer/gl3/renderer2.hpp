#ifndef RENDERER2_HPP
#define RENDERER2_HPP

#include <array>

// VertexLayout
#include <vertexlayout.hpp>
// Buffer
#include <buffer.hpp>
// Texture
#include <texture.hpp>


class Renderer
{
public:
	Texture2D createTexture2D(
		glm::ivec2 size,
        unsigned int numMipLevels,
        ElementFormat pixelFormat,
        const void* data);

	TextureCubeMap createTextureCubeMap(
		glm::ivec2 size,
        unsigned int numMipLevels,
        ElementFormat pixelFormat,
        const void* faceData[6]);

private:
};

 
#endif /* end of include guard: RENDERER2_HPP */