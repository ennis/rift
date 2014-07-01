#include <dummytexture.hpp>


CDummyTexture2D::CDummyTexture2D(Texture2DDesc &desc)
{}

CDummyTexture2D::~CDummyTexture2D()
{}

/*void CDummyTexture::update(glm::ivec3 const &coords, glm::ivec3 const &size, void *data)
{}*/

void CDummyTexture2D::destroy()
{
	delete this;
}