#include <texture.hpp>

Texture2D::Texture2D(
	glm::ivec2 size_,
	unsigned int numMipLevels_,
	ElementFormat pixelFormat_,
	const void *data
	) : Texture(gl::TEXTURE_2D, numMipLevels_, pixelFormat_), size(size_)
{
	gl::GenTextures(1, &id);
	const auto &pf = getElementFormatInfoGL(pixelFormat);
	if (gl::exts::var_EXT_direct_state_access) {
		gl::TextureStorage2DEXT(id, gl::TEXTURE_2D, numMipLevels, pf.internalFormat, size.x, size.y);
	}
	else {
		gl::BindTexture(gl::TEXTURE_2D, id);
		gl::TexStorage2D(gl::TEXTURE_2D, numMipLevels, pf.internalFormat, size.x, size.y);
	}

	if (data)
	{
		update(0, { 0, 0 }, size, data);
	}
}

void Texture2D::update(
	unsigned int mipLevel,
	glm::ivec2 offset, 
	glm::ivec2 size, 
	const void *data
	)
{
	const auto &pf = getElementFormatInfoGL(pixelFormat);
	if (gl::exts::var_EXT_direct_state_access)
	{
		gl::TextureSubImage2DEXT(
			id,
			gl::TEXTURE_2D,
			mipLevel,
			offset.x,
			offset.y,
			size.x,
			size.y,
			pf.externalFormat,
			pf.type,
			data);
	}
	else
	{
		gl::BindTexture(gl::TEXTURE_2D, id);
		gl::TexSubImage2D(
			gl::TEXTURE_2D,
			mipLevel,
			offset.x,
			offset.y,
			size.x,
			size.y,
			pf.externalFormat,
			pf.type,
			data);
		gl::BindTexture(gl::TEXTURE_2D, 0);
	}
}

Texture1D::Texture1D(
	unsigned int size_,
	unsigned int numMipLevels_,
	ElementFormat pixelFormat_
	) : Texture(gl::TEXTURE_1D, numMipLevels_, pixelFormat_), size(size_)
{
	gl::GenTextures(1, &id);
	const auto &pf = getElementFormatInfoGL(pixelFormat);
	if (gl::exts::var_EXT_direct_state_access) {
		gl::TextureStorage1DEXT(id, gl::TEXTURE_1D, numMipLevels, pf.internalFormat, size);
	}
	else {
		gl::BindTexture(gl::TEXTURE_1D, id);
		gl::TexStorage1D(gl::TEXTURE_1D, numMipLevels, pf.internalFormat, size);
	}
}

void Texture1D::update(
	unsigned int mipLevel,
	unsigned int offset,
	unsigned int size,
	const void *data)
{
	const auto &pf = getElementFormatInfoGL(pixelFormat);
	if (gl::exts::var_EXT_direct_state_access)
	{
		gl::TextureSubImage1DEXT(id, gl::TEXTURE_1D, mipLevel, offset, size, pf.externalFormat, pf.type, data);
	}
	else
	{
		gl::BindTexture(bindingPoint, id);
		gl::TexSubImage1D(bindingPoint, mipLevel, offset, size, pf.externalFormat, pf.type, data);
		gl::BindTexture(bindingPoint, 0);
	}
}


TextureCubeMap::TextureCubeMap(
	glm::ivec2 size_,
	unsigned int numMipLevels_,
	ElementFormat pixelFormat_,
	const void* faceData[6]
	) : Texture(gl::TEXTURE_CUBE_MAP, numMipLevels_, pixelFormat_), size(size_)
{
	gl::GenTextures(1, &id);
	const auto &pf = getElementFormatInfoGL(pixelFormat);
	if (gl::exts::var_EXT_direct_state_access) {
		gl::TextureStorage2DEXT(id, gl::TEXTURE_CUBE_MAP, numMipLevels, pf.internalFormat, size.x, size.y);
	}
	else {
		gl::BindTexture(gl::TEXTURE_CUBE_MAP, id);
		gl::TexStorage2D(gl::TEXTURE_CUBE_MAP, numMipLevels, pf.internalFormat, size.x, size.y);
	}

	for (int i = 0; i < 6; ++i) {
		update(i, 0, { 0, 0 }, size, faceData[i]);
	}
}

void TextureCubeMap::update(
	unsigned int face,
	unsigned int mipLevel,
	glm::ivec2 offset,
	glm::ivec2 size,
	const void *data)
{
	const auto &pf = getElementFormatInfoGL(pixelFormat);
	if (gl::exts::var_EXT_direct_state_access)
	{
		gl::TextureSubImage2DEXT(
			id,
			gl::TEXTURE_CUBE_MAP_POSITIVE_X + face,
			mipLevel,
			offset.x,
			offset.y,
			size.x,
			size.y,
			pf.externalFormat,
			pf.type,
			data);
	}
	else
	{
		gl::BindTexture(gl::TEXTURE_CUBE_MAP, id);
		gl::TexSubImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_X + face,
			mipLevel,
			offset.x,
			offset.y,
			size.x,
			size.y,
			pf.externalFormat,
			pf.type,
			data);
		gl::BindTexture(gl::TEXTURE_CUBE_MAP, 0);
	}
}