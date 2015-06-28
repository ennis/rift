#include <rendering/opengl4.hpp>
#include <mesh_data.hpp>
#include <glm/gtc/packing.hpp>

namespace 
{
	// packed vertex layout
	struct PackedVertex
	{
		glm::vec3 pos;
		glm::uint32 norm; // packSnorm3x10_1x2
		glm::uint32 tg; // packSnorm3x10_1x2
		glm::uint32 uv0;	// packUnorm2x16
	};
}

std::unique_ptr<Mesh> createMesh(GraphicsContext &gc, MeshData &data)
{
	auto ptr = std::make_unique<Mesh>();
	auto nv = data.vertices.size();
	auto ni = data.indices.size();
	ptr->vbo = gc.createBuffer(gl::ARRAY_BUFFER, nv*sizeof(PackedVertex));
	ptr->ibo = gc.createBuffer(gl::ELEMENT_ARRAY_BUFFER, ni * 2);
	ptr->nbvertex = nv;
	ptr->nbindex = ni;
	auto vbo_ptr = (PackedVertex*)ptr->vbo->ptr;
	for (auto i = 0u; i < nv; ++i) {
		vbo_ptr[i].pos = data.vertices[i];
		vbo_ptr[i].norm = glm::packSnorm3x10_1x2(glm::vec4(data.normals[i], 0.0f));
		vbo_ptr[i].tg = glm::packSnorm3x10_1x2(glm::vec4(data.tangents[i], 0.0f));
		vbo_ptr[i].uv0 = glm::packUnorm2x16(data.uv[0][i]);
	}
	auto ibo_ptr = (uint16_t*)ptr->ibo->ptr;
	for (auto i = 0u; i < ni; ++i)
		ibo_ptr[i] = data.indices[i];
	ptr->submeshes = data.submeshes;
	return std::move(ptr);
}

Mesh *loadMeshAsset(AssetDatabase &assetDb, GraphicsContext &gc, std::string assetId)
{
	return assetDb.loadAsset<Mesh>(assetId, [&]{
		std::ifstream fileIn(assetId, std::ios::binary);
		MeshData data;
		data.loadFromStream(fileIn); 
		return std::move(createMesh(gc, data));
	});
}