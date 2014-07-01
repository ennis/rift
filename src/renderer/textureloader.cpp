#include <textureloader.hpp>
#include <log.hpp>
#include <unimplemented.hpp>
#include <imageloader.hpp>


class Texture2DLoader : public ResourceLoader
{
public:
	void *load(std::string key);
	void destroy(std::string const &key, void *resource);

private:
};

void *Texture2DLoader::load(std::string key)
{
	glm::ivec2 size;
	int n;
	// load texture data
	std::unique_ptr<uint8_t> data = loadImageDataFromFile(key.c_str(), size, n);
	// create texture
	Texture2DDesc td;
	td.format = PixelFormat::R8G8B8A8;
	td.size = size;
	td.numMipMapLevels = 1;
	CTexture2D *tex = CRenderer::getInstance().createTexture2D(td, data.get());
	return tex;
}

void Texture2DLoader::destroy(std::string const &key, void *resource)
{
	CTexture2D *tex = static_cast<CTexture2D*>(resource);
	tex->destroy();
}


CTexture2DRef loadTexture2DFromFile(std::string path)
{
	return ResourceManager::getInstance().load<CTexture2D>(path, std::unique_ptr<ResourceLoader>(new Texture2DLoader()));
}

CTextureCubeMapRef loadTextureCubeMapFromFile(
	std::string positiveX, 
	std::string negativeX,
	std::string positiveY,
	std::string negativeY,
	std::string positiveZ,
	std::string negativeZ)
{
	class CubeMapLoader : public ResourceLoader
	{
	public:
		void *load(std::string key) {
			throw std::logic_error("TextureCubeMapLoader");
		}

		void destroy(std::string const &key, void *resource) {
			static_cast<CTextureCubeMap*>(resource)->destroy();
		}

	private:
	};

	// not managed for now
	glm::ivec2 sizePX, sizeNX, sizePY, sizeNY, sizePZ, sizeNZ;
	int n;
	auto dataPX = loadImageDataFromFile(positiveX.c_str(), sizePX, n);
	auto dataNX = loadImageDataFromFile(negativeX.c_str(), sizeNX, n);
	auto dataPY = loadImageDataFromFile(positiveY.c_str(), sizePY, n);
	auto dataNY = loadImageDataFromFile(negativeY.c_str(), sizeNY, n);
	auto dataPZ = loadImageDataFromFile(positiveZ.c_str(), sizePZ, n);
	auto dataNZ = loadImageDataFromFile(negativeZ.c_str(), sizeNZ, n);

	assert((sizePX == sizeNX)
		&& (sizePX == sizePY)
		&& (sizePX == sizeNY)
		&& (sizePX == sizePZ)
		&& (sizePX == sizeNZ));

	TextureCubeMapDesc desc;
	desc.format = PixelFormat::R8G8B8A8;
	desc.numMipMapLevels = 1;		// TODO specify this somewhere
	desc.size = sizePX;

	const void *initialData[6] = { dataPX.get(), dataNX.get(), dataPY.get(), dataNY.get(), dataPZ.get(), dataNZ.get() };

	auto tex = CRenderer::getInstance().createTextureCubeMap(desc, initialData);
	return make_handle<CTextureCubeMap>(tex, std::unique_ptr<ResourceLoader>(new CubeMapLoader()));
}

