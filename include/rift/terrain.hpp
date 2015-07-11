#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <image.hpp>
#include <array>
#include <scene_renderer.hpp>
#include <utils/small_vector.hpp>

static const int kMaxTerrainLodLevel = 16;

struct TerrainParams
{
	glm::mat4 modelMatrix;
	// size of the heightmap in pixels
	glm::vec2 heightmapSize;
	// heightmap vertical scale in meters/unit
	float verticalScale;
	// texture scale
	float flatTextureScale;
	float slopeTextureScale;
};

struct TerrainPatchParams
{
	// position of the grid in world space (in meters)
	glm::vec2 patchOffset;
	// size of the grid (in meters, scale)
	float patchScale;
	// LOD level
	int lodLevel;
};

struct Terrain
{
	double getHeight(double x, double y);
	uint16_t getHeightRaw(int x, int y);

	Image *heightmap;
	ImageView<uint16_t> heightmapView;
	Texture2D::Ptr heightTexture;
	Texture2D *flatTexture;
	float flatTextureScale;
	Texture2D *slopeTexture;
	float slopeTextureScale;
	float verticalScale;
	int log2Size;
	util::small_vector<float, kMaxTerrainLodLevel> LodRanges;
	int patchGridSize;
	int patchNumVertices;
	int patchNumIndices;
	Buffer::Ptr gridVb;
	Buffer::Ptr gridIb;
	Buffer::Ptr terrainParams;
};

struct TerrainInit
{
	Image *heightmap;
	Texture2D *flatTexture;
	float flatTextureScale;
	Texture2D *slopeTexture;
	float slopeTextureScale;
	float verticalScale;
};

std::unique_ptr<Terrain> createTerrain(GraphicsContext &gc, const TerrainInit &init);


#endif