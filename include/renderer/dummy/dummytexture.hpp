#ifndef DUMMYTEXTURE_HPP
#define DUMMYTEXTURE_HPP

#include <texture.hpp>

struct CDummyTexture2D : public CTexture2D
{
	CDummyTexture2D(Texture2DDesc &desc);
	~CDummyTexture2D();

	//void update(glm::ivec3 const &coords, glm::ivec3 const &size, void *data);
	void destroy();
};

#endif