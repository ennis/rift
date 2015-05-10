#ifndef FONT_HPP
#define FONT_HPP

#include <rendering/opengl4/opengl4.hpp>
#include <image.hpp>
#include <unordered_map>
#include <memory>

//=============================================================================
class Font
{
public:
	using Ptr = std::unique_ptr < Font > ;

	//--------------------------
	struct Glyph {
		Glyph(
			unsigned int x_,
			unsigned int y_,
			unsigned int width_,
			unsigned int height_,
			int xOffset_,
			int yOffset_,
			int xAdvance_,
			unsigned int page_) : 
			x(x_), 
			y(y_),
			width(width_), 
			height(height_), 
			xOffset(xOffset_), 
			yOffset(yOffset_), 
			xAdvance(xAdvance_),
			page(page_)
		{}
		// texture coordinates
		unsigned int x;
		unsigned int y;
		// width, height of the glyph
		unsigned int width;
		unsigned int height;
		int xOffset;
		int yOffset;
		int xAdvance;
		unsigned int page;
	};

	// do not use
	Font() = default;
	~Font();

	struct Metrics {
		unsigned int height;
		unsigned int baseline;
		unsigned int scaleW;
		unsigned int scaleH;
	};

	Metrics const &getMetrics() const {
		return metrics;
	}

	bool getGlyph(char32_t ch, Glyph const *& glyph) const {
		auto p = glyphs.find(ch);
		if (p != glyphs.end()) {
			glyph = &p->second;
			return true;
		} else {
			glyph = nullptr;
			return false;
		}
	}

	const Image &getTextureData(unsigned int page) const 
	{
		return data;
	}

	const gl4::Texture2D &getTexture() const
	{
		return *tex;
	}

	static Ptr loadFromFile(const char *fontFilePath);

private:
	Metrics metrics;
	Image data;
	gl4::Texture2D::Ptr tex;
	std::unordered_map<char32_t, Glyph> glyphs;
	// TODO kerning map
};

#endif
