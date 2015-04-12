#include <terrain.hpp>
#include <limits>
#include <glm/gtc/packing.hpp>
#include <gl4/effect.hpp>
#include <transform.hpp>
#include <boundingbox.hpp>

namespace
{
	struct TerrainUniforms
	{
		glm::mat4 modelMatrix;
		// size of the heightmap in pixels
		glm::vec2 heightmapSize;
		// heightmap vertical scale in meters/unit
		float heightmapScale;
		// texture scale
		float flatTextureScale;
		float slopeTextureScale;
	};

	struct TerrainPatchUniforms
	{
		// position of the grid in world space (in meters)
		glm::vec2 patchOffset;
		// size of the grid (in meters, scale)
		float patchScale;
		// LOD level
		int lodLevel;
	};

	constexpr auto max_terrain_patches = 2048;
}

//=============================================================================
Terrain::Terrain(
	Image heightmapData,
	Texture2D *slopeTexture,
	Texture2D *flatTexture) :
	patch_grid_size(32),
	hm_image(heightmapData),
	slope_tex(slopeTexture),
	flat_tex(flatTexture),
	hm_vert_scale(100.f)
{
	initHeightmap();
	initEffect();
	initGrid();
	calculateLodRanges();
}

//=============================================================================
float Terrain::getHeight(const glm::ivec2 &position)
{
	auto p = glm::clamp(position, glm::ivec2{ 0, 0 }, glm::ivec2{ hm_size.x - 1, hm_size.y - 1 });
	return hm_vert_scale * glm::unpackSnorm1x16(hm_view(p.x, p.y));
}

//=============================================================================
void Terrain::initHeightmap()
{
	hm_view = hm_image.getImageView(0, 0).viewAs<uint16_t>();
	hm_size = hm_view.size();
	hm_tex = Texture2D::create(hm_size, 1, ElementFormat::Unorm16, hm_view.data());
	log2_hm_size = std::max(1, int(std::log2(std::max(hm_size.x / patch_grid_size, 1))) + 1);
	// compute normals
	// TODO more compact format
	hm_normals = Image(ElementFormat::Float3, glm::ivec3(hm_size, 1.f), 1);
	hm_normals_view = hm_normals.getImageView().viewAs<glm::vec3>();
	LOG << "Calulating terrain normal map...";
	for (int x = 0; x < hm_size.x; ++x) {
		for (int y = 0; y < hm_size.y; ++y) {
			// skewed 5-point stencil
			float h00 = getHeight({ x, y });
			float h01 = getHeight({ x + 1, y });
			float h10 = getHeight({ x, y + 1 });
			float h11 = getHeight({ x + 1, y + 1 });
			float gx = 0.5f * (h11 - h00 + h01 - h10);
			float gy = 0.5f * (h00 - h11 + h01 - h10);
			glm::vec3 n{ -gx, 1.414213f, -gy };	// sqrt(2)/2 ???
			n = glm::normalize(n);
			hm_normals_view(x, y) = n;
		}
	}

	// create texture
	hm_normals_tex = hm_normals.convertToTexture2D();
}

//=============================================================================
void Terrain::initEffect()
{
	gl4::Effect::Ptr effect = gl4::Effect::loadFromFile("resources/shaders/terrain.glsl");
	shader = effect->compileShader();
}

//=============================================================================
void Terrain::initGrid()
{
	// number of vertices 
	int gs = patch_grid_size + 1;
	patch_num_vertices = gs*gs;
	patch_num_indices = patch_grid_size * patch_grid_size * 6;
	// create patch grid
	std::vector<float> vertices(patch_num_vertices * 2);
	std::vector<uint16_t> indices(patch_num_indices);
	int p = 0;
	float xp = 0.f, yp = 0.f;
	float dd = 1.f / patch_grid_size;
	for (int i = 0; i<gs; ++i) {
		for (int j = 0; j<gs; ++j) {
			vertices[(i*gs + j) * 2 + 0] = i*dd;
			vertices[(i*gs + j) * 2 + 1] = j*dd;
			if ((i<patch_grid_size) && (j<patch_grid_size)) {
				indices[p++] = i*gs + j + 1;
				indices[p++] = i*gs + j;
				indices[p++] = (i + 1)*gs + j;
				indices[p++] = i*gs + j + 1;
				indices[p++] = (i + 1)*gs + j;
				indices[p++] = (i + 1)*gs + j + 1;
			}
		}
	}
	// create VB and IB
	patch_grid_vb = Buffer::create(
		vertices.size() * sizeof(float),
		ResourceUsage::Static,
		BufferUsage::VertexBuffer,
		vertices.data());

	patch_grid_vb = Buffer::create(
		indices.size() * sizeof(uint16_t),
		ResourceUsage::Static,
		BufferUsage::IndexBuffer,
		indices.data());

	input_layout = InputLayout::create(1, { { ElementFormat::Float2, 0 } });

	terrain_uniforms = Stream::create(BufferUsage::ConstantBuffer, sizeof(TerrainUniforms), 3);
	terrain_patch_uniforms = Stream::create(BufferUsage::ConstantBuffer, max_terrain_patches*sizeof(TerrainPatchUniforms), 3);

	TerrainUniforms uniforms;
	uniforms.modelMatrix = Transform().scale(1.f).toMatrix();
	uniforms.heightmapSize = glm::vec2(hm_size.x, hm_size.y);
	uniforms.heightmapScale = hm_vert_scale;
	uniforms.flatTextureScale = 1.0f;
	uniforms.slopeTextureScale = 1.0f;
	terrain_uniforms->write(uniforms);
}


//=============================================================================
void Terrain::renderSelection(SceneRenderContext &context)
{
	for (int i = 0; i < selected_nodes.size(); ++i) {
		const auto &node = selected_nodes[i];
		renderNode(context, node);
	}
}

void Terrain::renderNode(SceneRenderContext &context, Node const &node)
{
	auto &renderQueue = *context.opaqueRenderQueue;

	renderQueue.beginCommand();
	renderQueue.setVertexBuffers({ patch_grid_vb->getDescriptor() }, *input_layout);
	renderQueue.setIndexBuffer(patch_grid_ib->getDescriptor());
	renderQueue.setShader(*shader);
	renderQueue.setTexture2D(0, *hm_tex, SamplerDesc{});
	renderQueue.setTexture2D(1, *hm_normals_tex, SamplerDesc{});
	renderQueue.setTexture2D(2, *slope_tex, SamplerDesc{});
	renderQueue.setTexture2D(3, *flat_tex, SamplerDesc{});

	TerrainPatchUniforms patchUniforms;
	patchUniforms.lodLevel = node.lod;
	patchUniforms.patchOffset = glm::vec2(node.x, node.y);
	patchUniforms.patchScale = static_cast<float>(node.size);
	terrain_patch_uniforms->write(patchUniforms);

	renderQueue.setUniformBuffers({
		context.sceneDataCB,
		terrain_uniforms->getDescriptor(),
		terrain_patch_uniforms->getDescriptor() });

	renderQueue.drawIndexed(PrimitiveType::Triangle, 0, patch_num_indices, 0, 0, 1);
}

//=============================================================================
void Terrain::nodeLodSelect(
	const Node &node,
	const glm::vec3 &eye,
	unsigned currentLodLevel)
{
	// TODO: do frustrum culling here
	if (currentLodLevel == 0) {
		addNodeToSelection(node);
		return;
	}
	// if the current LOD level is higher that the maximum LOD 
	// the node must be subdivided anyway
	if (currentLodLevel < lod_ranges.size()) {
		if (!nodeIntersectLodRange(node, eye, currentLodLevel)) {
			addNodeToSelection(node);
			return;
		}
	}
	currentLodLevel--;
	// subdiv 4x
	int half = node.size / 2;
	Node NW, NE, SW, SE;
	NW.x = node.x;
	NW.y = node.y;
	NW.size = half;
	NW.lod = currentLodLevel;
	NE.x = node.x + half;
	NE.y = node.y;
	NE.size = half;
	NE.lod = currentLodLevel;
	SW.x = node.x;
	SW.y = node.y + half;
	SW.size = half;
	SW.lod = currentLodLevel;
	SE.x = node.x + half;
	SE.y = node.y + half;
	SE.size = half;
	SE.lod = currentLodLevel;
	nodeLodSelect(NW, eye, currentLodLevel);
	nodeLodSelect(NE, eye, currentLodLevel);
	nodeLodSelect(SW, eye, currentLodLevel);
	nodeLodSelect(SE, eye, currentLodLevel);
}

//=============================================================================
bool Terrain::nodeIntersectLodRange(
	const Node &node,
	const glm::vec3 &eye,
	unsigned lod)
{
	// check all corners for intersection
	AABB aabb;
	aabb.min.x = float(node.x);
	aabb.min.y = 0.f;	// TODO heightmap Y bounds
	aabb.min.z = float(node.y);
	aabb.max.x = float(node.x + node.size);
	aabb.max.y = 0.f;
	aabb.max.z = float(node.y + node.size);
	float range = lod_ranges[lod];
	return aabb.distanceFromPointSq(eye) <= range*range;
}

//=============================================================================
void Terrain::addNodeToSelection(const Node &node)
{
	selected_nodes.push_back(node);
}

//=============================================================================
void Terrain::clearNodeSelection()
{
	selected_nodes.clear();
}

//=============================================================================
void Terrain::calculateLodRanges()
{
	constexpr float lodRange = 2.5;
	const unsigned num_lod_levels =
		std::min(
			static_cast<unsigned>((1.f / log2(lodRange)) * log2(std::max(hm_size.x / patch_grid_size, 1))) + 1,
			kMaxLodLevel);

	LOG << "Terrain: " << num_lod_levels << " LOD levels";
	for (auto i = 0u; i < num_lod_levels; ++i) {
		lod_ranges[i] = static_cast<float>(patch_grid_size)* powf(lodRange, static_cast<float>(i));
		LOG << "LOD " << i << " up to " << lod_ranges[i];
	}
}
