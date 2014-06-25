#ifndef TEXTURELOADER_HPP
#define TEXTURELOADER_HPP

#include <resourcemanager.hpp>
#include <texture.hpp>
#include <renderer.hpp>

class TextureLoader : public ResourceLoader
{
public:
	TextureLoader(CRenderer &renderer);
	~TextureLoader();

	CResourceBase *load(std::string key);
	void destroy(std::string const &key, CResourceBase *resource);

private:
	CRenderer &mRenderer;
};

#endif