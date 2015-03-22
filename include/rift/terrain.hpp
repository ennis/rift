#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <renderer.hpp>
#include <renderable.hpp>
#include <image.hpp>

class Terrain
{
public:
	// TODO reference
	Terrain(Renderer &renderer, Image &&heightmapData);
	~Terrain();

	void render(RenderContext const &renderContext);

	// height receive the height of the terrain at a given position
	// return true if there was an error (out of boundaries...)
	bool getHeight(glm::vec2 pos, float &height);

	// normal receive the normal of the terrain at a given position
	// return true if there was an error (out of boundaries...)
	bool Terrain::getNormal(glm::vec2 pos, glm::vec3 &normal);

private:
	Renderer &mRenderer;

	void init();
	void initHeightmap();
	void initShader();
	void initGrid();
	void calculateLodRanges();

	struct Node
	{
		// in heightmap pixels
		int x, y, size, lod;
	};

	void renderSelection(RenderContext const &renderContext);
	void renderNode(RenderContext const &renderContext, Node const &node);

	void nodeLodSelect(
		Node const &node, 
		glm::vec3 const &eye, 
		int currentLodLevel);

	bool nodeIntersectLodRange(Node const &node, glm::vec3 const &eye, int lod);

	void addNodeToSelection(Node const &node);
	void clearNodeSelection();


	// Selected nodes
	static const int kMaxNodes = 8192;
	Node mSelectedNodes[kMaxNodes];
	int mNumSelectedNodes;

	// Patch data
	int mPatchGridSize;
	int mPatchNumVertices;
	int mPatchNumIndices;
	VertexBuffer *mPatchGridVB; // unique_ptr
	IndexBuffer *mPatchGridIB; // unique_ptr
	VertexLayout *mVertexLayout;

	// Terrain heightmap data & texture
	Image mHeightmapData;
	ImageView<uint16_t> mHeightmapView;
	Texture2D *mHeightmapTexture; // unique_ptr

	// Heightmap vertical scale
	float mHeightmapVerticalScale;
	// Heightmap size in pixels
	glm::ivec2 mHeightmapSize;
	int mLog2HeightmapSize;

	// LOD ranges
	int mNumLodLevels;
	static const int kMaxLodLevel = 16;
	float mLodRanges[kMaxLodLevel];

	// shader
	Shader *mShader; // unique_ptr
};

#endif