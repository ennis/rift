#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <gl4/renderer.hpp>	
#include <image.hpp>
#include <array>
#include <scene.hpp>
#include <small_vector.hpp>

class Terrain
{
public:
	Terrain(
		Image heightmapData,
		Texture2D *slopeTexture,
		Texture2D *flatTexture);

	void render(SceneRenderContext &context);
	float getHeight(const glm::ivec2 &position);

private:
	void initHeightmap();
	void initEffect();
	void initGrid();

	void calculateLodRanges();

	struct Node
	{
		// in heightmap pixels
		int x, y, size, lod;
	};

	void renderSelection(SceneRenderContext &context);
	void renderNode(SceneRenderContext &context, const Node &node);

	void nodeLodSelect(
		const Node &node, 
		const glm::vec3 &eye, 
		unsigned currentLodLevel);

	bool nodeIntersectLodRange(
		const Node &node, 
		const glm::vec3 &eye, 
		unsigned lod);

	void addNodeToSelection(const Node &node);
	void clearNodeSelection();

	// Selected nodes
	std::vector<Node> selected_nodes;
	int patch_grid_size;
	int patch_num_vertices;
	int patch_num_indices;
	Buffer::Ptr patch_grid_vb; 
	Buffer::Ptr patch_grid_ib; 
	InputLayout::Ptr input_layout;
	// Terrain heightmap data & texture
	Image hm_image;
	ImageView<uint16_t> hm_view;
	// Terrain normal map
	Image hm_normals;
	ImageView<glm::vec3> hm_normals_view;
	Texture2D::Ptr hm_tex; 
	Texture2D::Ptr hm_normals_tex;
	// resource_ptr
	Texture2D *slope_tex;
	Texture2D *flat_tex;
	// Heightmap vertical scale
	float hm_vert_scale;
	// Heightmap size in pixels
	glm::ivec2 hm_size;
	int log2_hm_size;
	// LOD ranges
	static const unsigned kMaxLodLevel = 16;
	util::small_vector<float, kMaxLodLevel> lod_ranges;
	Shader::Ptr shader;
	Shader::Ptr shader_wireframe;
	// shader parameters
	Buffer::Ptr terrain_uniforms;
};

#endif