#include <font.hpp>
#include <cstring>
#include <string>
#include <serialization.hpp>

//=============================================================================
Font::Font(Renderer &renderer, const char *fontFilePath)
{
	loadFromFile(renderer, fontFilePath);
}

//=============================================================================
Font::~Font()
{
	// TODO release textures
}

//=============================================================================
void Font::loadFromFile(Renderer &renderer, const char *fontFilePath)
{
	// binary file produced by BMFont
	std::ifstream fileIn(fontFilePath, std::ios::in | std::ios::binary);
	assert(fileIn.is_open());
	char sig[3];
	fileIn.read(sig, 3);
	if (sig[0] != 'B' || sig[1] != 'M' || sig[2] != 'F') {
		ERROR << "BMF: Invalid magic";
		assert(false);
	}
	// file version
	uint8_t version;
	fileIn.read((char*)&version, 1);
	// texture files
	std::vector<std::string> textureFileNames;
	// blocks
	while (true) {
		uint8_t blockType;
		fileIn.read((char*)&blockType, 1);
		if (fileIn.eof() || fileIn.fail()) break;
		uint32_t blockSize;
		fileIn.read((char*)&blockSize, 4);
		// file should not end here
		if (fileIn.eof() || fileIn.fail()) break;
		switch (blockType) {
		case 0x01:
			{
				// skip the block
				fileIn.ignore(blockSize);
			}
			break;
		case 0x02:
			{
				// common
				// lineHeight 2 uint
				read_u16le(fileIn, mMetrics.height);
				read_u16le(fileIn, mMetrics.baseline);
				read_u16le(fileIn, mMetrics.scaleW);
				read_u16le(fileIn, mMetrics.scaleH);
				read_u16le(fileIn, mNumGlyphPages);
				uint8_t flags;
				read_u8(fileIn, flags);
				// channel bits
				uint8_t ch[4];
				fileIn.read((char*)ch, 4);
				assert(!(fileIn.fail() || fileIn.eof()));
				// encoded glyph + outline in alpha channel
				assert(ch[0] == 2);
				assert(mNumGlyphPages < kMaxGlyphPages);
			}
			break;
		case 0x03:
			{
				// file names
				std::string fileName;
				std::getline(fileIn, fileName, char(0));
				assert(!(fileIn.fail() || fileIn.eof()));
				textureFileNames.push_back(fileName);
				LOG << "Texture file: " << fileName.c_str();
			}
			break;
		case 0x04:
			{
				// characters
				int numChars = blockSize / 20;
				for (int i = 0; i < numChars; ++i) {
					char32_t id;
					unsigned int x, y, width, height, page;
					int xOffset, yOffset, xAdvance;
					read_u32le(fileIn, id);
					read_u16le(fileIn, x);
					read_u16le(fileIn, y);
					read_u16le(fileIn, width);
					read_u16le(fileIn, height);
					read_i16le(fileIn, xOffset);
					read_i16le(fileIn, yOffset);
					read_i16le(fileIn, xAdvance);
					read_u8(fileIn, page);
					// skip channel
					fileIn.ignore(1);
					assert(!(fileIn.fail() || fileIn.eof()));
					mGlyphs.emplace(
						std::make_pair(
							id, 
							Glyph(
								x, y, width, height, 
								xOffset, yOffset, xAdvance, 
								page)));
				}
			}
		default:
			WARNING << "BMF: ignored block type";
			// skip unknown block
			fileIn.ignore(blockSize);
			break;
		}
	}
	// load the textures
	for (unsigned int i = 0; i < mNumGlyphPages; ++i) {
		std::string sp(fontFilePath);
		int l = sp.find_last_of('/');
		if (l != std::string::npos) {
			sp.erase(sp.cbegin() + l + 1, sp.cend());
		}
		sp += textureFileNames[i];
		LOG << "Loading " << sp.c_str();
		auto &texData = mGlyphPages[i].data; 
		texData.loadFromFile(sp.c_str());
		mGlyphPages[i].tex = texData.convertToTexture2D(renderer);
	}
}

