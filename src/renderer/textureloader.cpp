#include <textureloader.hpp>
#include <log.hpp>
#include <unimplemented.hpp>
#include <imageloader.hpp>


TextureLoader::TextureLoader(CRenderer &renderer) : mRenderer(renderer)
{
}

TextureLoader::~TextureLoader()
{

}

CResourceBase *TextureLoader::load(std::string key)
{
	glm::ivec2 size;
	int n;
	// load texture data
	std::unique_ptr<uint8_t> data = loadImageDataFromFile(key.c_str(), size, n);
	// create texture
	TextureDesc td;
	td.format = PixelFormat::R8G8B8A8;
	td.size = glm::ivec3(size.x, size.y, 1);
	td.textureType = TextureType::Texture2D;
	td.numMipMapLevels = 0;
	CTexture *tex = mRenderer.createTexture(td);
	tex->update(glm::ivec3(0, 0, 0), td.size, data.get());
	return tex;
}

void TextureLoader::destroy(std::string const &key, CResourceBase *resource)
{
	CTexture *tex = static_cast<CTexture*>(resource);
	tex->deleteResource();
	ERROR << "TODO";
}
