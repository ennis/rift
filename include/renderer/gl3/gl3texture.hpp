#ifndef GL3TEXTURE_HPP
#define GL3TEXTURE_HPP

#include <renderer.hpp>
#include <opengl.hpp>

struct CGL3Texture : public CTexture
{
	CGL3Texture(TextureDesc &desc);
	~CGL3Texture();

	void initialize();
	void setActive(int textureUnit);

	void update(glm::ivec3 const &coords, glm::ivec3 const &size, void *data);
	void deleteResource();

	GLuint mObj;
};

#endif