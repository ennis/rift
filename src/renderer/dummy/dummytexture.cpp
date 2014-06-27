#include <dummytexture.hpp>


CDummyTexture::CDummyTexture(TextureDesc &desc)
{}

CDummyTexture::~CDummyTexture()
{}

void CDummyTexture::update(glm::ivec3 const &coords, glm::ivec3 const &size, void *data)
{}

void CDummyTexture::destroy()
{
	delete this;
}