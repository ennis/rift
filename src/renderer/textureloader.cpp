#include <textureloader.hpp>
#include <log.hpp>
#include <unimplemented.hpp>
#include <imageloader.hpp>


class TextureLoader : public ResourceLoader
{
public:
	void *load(std::string key);
	void destroy(std::string const &key, void *resource);

private:
};

void *TextureLoader::load(std::string key)
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
	CTextureRef tex = CRenderer::getInstance().createTexture(td);
	auto texPtr = tex.lock();
	texPtr->update(glm::ivec3(0, 0, 0), td.size, data.get());
	tex.unlock();
	return tex.mControlBlock->mData;
}

void TextureLoader::destroy(std::string const &key, void *resource)
{
	CTexture *tex = static_cast<CTexture*>(resource);
	//tex->deleteResource();
	ERROR << "TODO";
}


CTextureRef loadTextureFromFile(std::string path)
{
	return ResourceManager::getInstance().load<CTexture>(path, std::unique_ptr<ResourceLoader>(new TextureLoader()));
}
