#ifndef FONT_HPP
#define FONT_HPP

#include <renderer.hpp>
#include <texturedata.hpp>
#include <unordered_map>

//=============================================================================
class Font
{
public:
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
	//--------------------------
	Font() = default;
	Font(Renderer &renderer, const char *fontFilePath);
	~Font();
	void loadFromFile(Renderer &renderer, const char *fontFilePath);
	struct Metrics {
		unsigned int height;
		unsigned int baseline;
		unsigned int scaleW;
		unsigned int scaleH;
	};
	Metrics const &metrics() const {
		return mMetrics;
	}
	unsigned int getNumGlyphPages() const {
		return mNumGlyphPages;
	}
	bool getGlyph(char32_t ch, Glyph const *& glyph) const {
		auto p = mGlyphs.find(ch);
		if (p != mGlyphs.end()) {
			glyph = &p->second;
			return true;
		} else {
			glyph = nullptr;
			return false;
		}
	}
	TextureData const &getTextureData(unsigned int page) const {
		assert(page < mNumGlyphPages);
		return mGlyphPages[page].data;
	}
	Texture *getTexture(unsigned int page) {
		assert(page < mNumGlyphPages);
		return mGlyphPages[page].tex;
	} 
private:
	Metrics mMetrics;
	// number of pages
	unsigned int mNumGlyphPages = 0;
	static const unsigned int kMaxGlyphPages = 8;
	struct GlyphPage {
		TextureData data;
		Texture *tex;
	} mGlyphPages[kMaxGlyphPages];
	// glyph map
	std::unordered_map<char32_t, Glyph> mGlyphs;
	// TODO kerning map
};

#endif
