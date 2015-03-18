#include <renderer.hpp>

namespace
{
	struct VertexType0
	{
		glm::vec2 pos;
		glm::uint32 tex;
	};

	struct VertexType1
	{
		glm::vec3 pos;
		glm::vec3 norm;
		glm::vec3 tg;
		glm::vec3 bitg;
		glm::vec2 tex;
	};

	struct VertexType2
	{
		glm::vec3 pos;
		glm::uint32 norm; // packSnorm3x10_1x2
		glm::uint32 tg; // packSnorm3x10_1x2
		glm::uint32 bitg;	// packSnorm3x10_1x2
		glm::uint32 tex;	// packUnorm2x16
	};
}

Mesh::Ptr Mesh::loadFromArchive(serialization::IArchive &ar)
{
	using namespace serialization;

	std::vector<Submesh> submeshes;
	unsigned int num_submeshes, num_vertices, num_indices, layout, version;

	ar >> read8(version) 
		>> read8(layout)
	    >> read16(num_submeshes) 
	    >> num_vertices 
	    >> num_indices;

	assert(version == 3);
	assert(layout == 1);
	assert(num_submeshes < 65536);
	assert(num_vertices < 40*1024*1024);
	assert(num_indices < 40*1024*1024);

	submeshes.resize(num_submeshes);
	for (auto i = 0u; i < num_submeshes; ++i) {
		ar >> submeshes[i].startVertex 
			>> submeshes[i].startIndex 
			>> submeshes[i].numVertices 
			>> read16(submeshes[i].numIndices);
	}

	std::vector<VertexType1> vs(num_vertices);
	std::vector<uint16_t> is(num_indices);
	for (auto i = 0u; i < num_vertices; ++i) {
		auto &v = vs[i];
		ar >> v.pos.x >> v.pos.y >> v.pos.z 
			>> v.norm.x >> v.norm.y >> v.norm.z
			>> v.tg.x >> v.tg.y >> v.tg.z
			>> v.bitg.x >> v.bitg.y >> v.bitg.z
			>> v.tex.x >> v.tex.y;
	}
	for (auto i = 0u; i < num_indices; ++i) {
		ar >> read16(is[i]);
	}

	// create mesh
	auto ptr = Mesh::create(
		PrimitiveType::Triangle,
		{ 
			{ElementFormat::Float3, ResourceUsage::Static},
			{ElementFormat::Float3, ResourceUsage::Static},
			{ElementFormat::Float3, ResourceUsage::Static},
			{ElementFormat::Float3, ResourceUsage::Static},
			{ElementFormat::Float2, ResourceUsage::Static}
		},
		num_vertices,
		vs.data(),
		num_indices,
		is.data());
	return ptr;
}