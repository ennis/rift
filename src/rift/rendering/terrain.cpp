#include <terrain.hpp>

double Terrain::getHeight(double x, double y)
{
	return 0;
}

uint16_t Terrain::getHeightRaw(int x, int y)
{
	return 0;
}


std::unique_ptr<Terrain> createTerrain(GraphicsContext &gc, const TerrainInit &init)
{
	auto ptr = std::make_unique<Terrain>();
	ptr->flatTexture = init.flatTexture;
	ptr->flatTextureScale = init.flatTextureScale;
	ptr->slopeTexture = init.slopeTexture;
	ptr->slopeTextureScale = init.slopeTextureScale;
	ptr->heightmap = init.heightmap;
	ptr->verticalScale = init.verticalScale;
	ptr->heightTexture = Texture2D::create(init.heightmap->getSize(), 1, ElementFormat::Unorm16x2, init.heightmap->getData());
	// patch creation
	{
		// patch grid size
		auto size = 16;
		auto s1 = size + 1;
		auto nv = s1 * s1;
		auto ni = (size * size) * 6;
		std::vector<glm::vec2> vertices(nv);
		std::vector<uint16_t> indices(ni);

		int p = 0;
		float dd = 1.0f / size;
		// lines
		for (int i = 0; i < (size + 1); ++i) {
			// columns
			for (int j = 0; j < (size + 1); ++j) {
				vertices[(i*s1 + j)].x = i*dd;
				vertices[(i*s1 + j)].y = j*dd;
				if ((i < size) && (j < size)) {
					indices[p++] = i*s1 + j + 1;
					indices[p++] = i*s1 + j;
					indices[p++] = (i + 1)*s1 + j;
					indices[p++] = i*s1 + j + 1;
					indices[p++] = (i + 1)*s1 + j;
					indices[p++] = (i + 1)*s1 + j + 1;
				}
			}
		}

		ptr->patchGridSize = size;
		ptr->patchNumVertices = nv;
		ptr->patchNumIndices = ni;
		ptr->gridVb = gc.createBuffer(gl::ARRAY_BUFFER, vertices.size() * 8, vertices.data());
		ptr->gridIb = gc.createBuffer(gl::ELEMENT_ARRAY_BUFFER, indices.size() * 2, indices.data());
	}

	auto heightmapSize = init.heightmap->getSize();
	ptr->log2Size = std::max(1, int(std::log2(std::max(heightmapSize.x / ptr->patchGridSize, 1))) + 1);
	// calculate LOD ranges
	float lodRange = 2.5;
	auto num_lod_levels = std::min(
			int((1.f / log2(lodRange)) * log2(std::max(heightmapSize.x / ptr->patchGridSize, 1))) + 1,
			kMaxTerrainLodLevel);
	for (auto i = 0u; i < num_lod_levels; ++i) {
		ptr->LodRanges.push_back(float(ptr->patchGridSize) * powf(lodRange, float(i)));
	}
	// setup uniform buffer
	TerrainParams terrainParams;
	terrainParams.flatTextureScale = init.flatTextureScale;
	terrainParams.slopeTextureScale = init.slopeTextureScale;
	terrainParams.verticalScale = init.verticalScale;
	terrainParams.modelMatrix = glm::mat4(1.0f);
	terrainParams.heightmapSize.x = float(heightmapSize.x);
	terrainParams.heightmapSize.y = float(heightmapSize.y);
	ptr->terrainParams = gc.createBuffer(gl::UNIFORM_BUFFER, sizeof(TerrainParams), &terrainParams);
}
