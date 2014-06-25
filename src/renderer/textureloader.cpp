#include <textureloader.hpp>
#include <log.hpp>
#include <unimplemented.hpp>


TextureLoader::TextureLoader(CRenderer &renderer) : mRenderer(renderer)
{

}

TextureLoader::~TextureLoader()
{}

CResourceBase *TextureLoader::load(std::string key)
{
	// TODO load from file...
	TextureDesc td;
	td.format = PixelFormat::R8G8B8A8;
	td.size = glm::ivec3(800, 600, 1);
	td.textureType = TextureType::Texture2D;
	td.numMipMapLevels = 0;
	return mRenderer.createTexture(td);
}

void TextureLoader::destroy(std::string const &key, CResourceBase *resource)
{
	CTexture *tex = static_cast<CTexture*>(resource);
	tex->deleteResource();
	ERROR << "TODO";
}
