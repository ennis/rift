#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <renderer2.hpp>	// Renderer
#include <buffer.hpp>	// Buffer
#include <renderable.hpp>
#include <image.hpp>
#include <effect.hpp>

#include <array>

class Terrain
{
public:
	Terrain(
		Renderer &renderer, 
		Image &&heightmapData,
		Texture2D *slopeTexture,
		Texture2D *flatTexture);

	Terrain(const Terrain &) = delete;
	Terrain &operator=(const Terrain &) = delete;
	// move-constructible
	Terrain(Terrain &&rhs);
	// nonmoveable
	Terrain &operator=(Terrain &&rhs) = delete;
	~Terrain();

	void render(const RenderContext &renderContext);
	float getHeight(const glm::ivec2 &position);

private:
	void initHeightmap(Renderer &renderer);
	void initEffect(Renderer &renderer);
	void initGrid(Renderer &renderer);

	void calculateLodRanges();

	struct Node
	{
		// in heightmap pixels
		int x, y, size, lod;
	};

	void renderSelection(const RenderContext &renderContext);
	void renderNode(const RenderContext &renderContext, const Node &node);

	void nodeLodSelect(
		Node const &node, 
		glm::vec3 const &eye, 
		int currentLodLevel);

	bool nodeIntersectLodRange(Node const &node, glm::vec3 const &eye, int lod);

	void addNodeToSelection(Node const &node);
	void clearNodeSelection();

	// Selected nodes
	std::vector<Node> mSelectedNodes;
	int mNumSelectedNodes;
	// Patch data
	int mPatchGridSize;
	int mPatchNumVertices;
	int mPatchNumIndices;
	Buffer mPatchGridVB; 
	Buffer mPatchGridIB; 
	VertexLayout mVertexLayout;
	// Terrain heightmap data & texture
	Image mHeightmapData;
	ImageView<uint16_t> mHeightmapView;
	// Terrain normal map
	Image mHeightmapNormals;
	ImageView<glm::vec3> mHeightmapNormalsView;
	Texture2D mHeightmapTexture; 
	Texture2D mHeightmapNormalTexture;
	Texture2D *mSlopeTexture;
	Texture2D *mFlatTexture;
	// Heightmap vertical scale
	float mHeightmapVerticalScale;
	// Heightmap size in pixels
	glm::ivec2 mHeightmapSize;
	int mLog2HeightmapSize;
	// LOD ranges
	int mNumLodLevels;
	static const int kMaxLodLevel = 16;
	std::array<float,kMaxLodLevel> mLodRanges;
	Effect mEffect;
	Shader *mShader;
};

#endif