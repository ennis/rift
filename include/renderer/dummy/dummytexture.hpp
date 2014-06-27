#ifndef DUMMYTEXTURE_HPP
#define DUMMYTEXTURE_HPP

#include <texture.hpp>

struct CDummyTexture : public CTexture
{
	CDummyTexture(TextureDesc &desc);
	~CDummyTexture();

	void update(glm::ivec3 const &coords, glm::ivec3 const &size, void *data);
	void destroy();
};

#endif