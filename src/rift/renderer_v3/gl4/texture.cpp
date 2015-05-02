#include <renderer.hpp>

namespace gl4
{
	TextureCubeMap::TextureCubeMap(
		glm::ivec2 size_,
		int numMipLevels_,
		ElementFormat pixelFormat_,
		const void* faceData[6]
		) :
		Texture(Texture::TexCubeMap),
		size(size_),
		format(pixelFormat_),
		glformat(getElementFormatInfoGL(pixelFormat_).internalFormat)
	{
		gl::GenTextures(1, &id);
		const auto &pf = getElementFormatInfoGL(pixelFormat_);
		if (gl::exts::var_EXT_direct_state_access) {
			gl::TextureStorage2DEXT(id, gl::TEXTURE_CUBE_MAP, numMipLevels_, glformat, size.x, size.y);
		}
		else {
			gl::BindTexture(gl::TEXTURE_CUBE_MAP, id);
			gl::TexStorage2D(gl::TEXTURE_CUBE_MAP, numMipLevels_, glformat, size.x, size.y);
		}

		for (int i = 0; i < 6; ++i) {
			if (faceData[i])
				update(i, 0, { 0, 0 }, size, faceData[i]);
		}
	}

	void TextureCubeMap::update(
		int face,
		int mipLevel,
		glm::ivec2 offset,
		glm::ivec2 size,
		const void *data
		)
	{
		const auto &pf = getElementFormatInfoGL(format);
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

	Texture2D::Texture2D(
		glm::ivec2 size_,
		int numMipLevels_,
		ElementFormat pixelFormat_,
		const void *data_
		) :
		Texture(Texture::Tex2D),
		size(size_),
		format(pixelFormat_),
		glformat(getElementFormatInfoGL(pixelFormat_).internalFormat)
	{
		GLuint tex;
		gl::GenTextures(1, &tex);
		const auto &pf = getElementFormatInfoGL(pixelFormat_);
		if (gl::exts::var_EXT_direct_state_access) {
			gl::TextureStorage2DEXT(tex, gl::TEXTURE_2D, numMipLevels_, glformat, size.x, size.y);
		}
		else {
			gl::BindTexture(gl::TEXTURE_2D, tex);
			gl::TexStorage2D(gl::TEXTURE_2D, numMipLevels_, glformat, size.x, size.y);
			gl::BindTexture(gl::TEXTURE_2D, 0);
		}

		id = tex;

		if (data_)
			update(0, { 0, 0 }, size, data_);
	}


	void Texture2D::update(
		int mipLevel,
		glm::ivec2 offset,
		glm::ivec2 size,
		const void *data
		)
	{
		const auto &pf = getElementFormatInfoGL(format);
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
}