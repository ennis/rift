#ifndef GL3TEXTURE_HPP
#define GL3TEXTURE_HPP

#include <renderer.hpp>
#include <opengl.hpp>
#include <texture.hpp>

struct CGL3Texture2D : public CTexture2D
{
	CGL3Texture2D(Texture2DDesc &desc);
	~CGL3Texture2D();

	void initialize();
	void setActive(int textureUnit);

	void update(int mipLevel, glm::ivec2 coords, glm::ivec2 size, const void *data) override;
	void destroy() override;

	GLuint mObj;
};

struct CGL3TextureCubeMap : public CTextureCubeMap
{
	CGL3TextureCubeMap(TextureCubeMapDesc &desc);
	~CGL3TextureCubeMap();

	void initialize();
	void setActive(int textureUnit);

	void update(int mipLevel, int face, glm::ivec2 coords, glm::ivec2 size, const void *data) override;
	void destroy() override;

	GLuint mObj;
};

#endif