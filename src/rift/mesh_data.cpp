#include <mesh_data.hpp>
#include <utils/binary_io.hpp>

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
		glm::vec3 norm;
		glm::vec3 tg;
		glm::vec3 bitg;
		glm::vec2 tex;
		glm::uint32 bone_ids;
		glm::vec4 bone_weights;
	};

	struct VertexType3
	{
		glm::vec3 pos;
		glm::uint32 norm; // packSnorm3x10_1x2
		glm::uint32 tg; // packSnorm3x10_1x2
		glm::uint32 bitg;	// packSnorm3x10_1x2
		glm::uint32 tex;	// packUnorm2x16
	};

	struct MeshDataHeader
	{
		uint8_t version;
		uint8_t layout;
		unsigned num_vertices;
		unsigned num_indices;
		std::vector<Submesh> submeshes;
	};

	void readMeshDataHeader(
		util::BinaryReader &ar, 
		MeshDataHeader &out)
	{
		unsigned num_submeshes;
		ar >> util::read8(out.version)
			>> util::read8(out.layout)
			>> util::read16(num_submeshes)
			>> out.num_vertices
			>> out.num_indices;

		assert(out.version == 3);
		assert(num_submeshes < 65536);
		assert(out.num_vertices < 40 * 1024 * 1024);
		assert(out.num_indices < 40 * 1024 * 1024);

		out.submeshes.resize(num_submeshes);
		for (auto i = 0u; i < num_submeshes; ++i) {
			if (out.layout == 5)
			{
				ar >> out.submeshes[i].startVertex
					>> out.submeshes[i].startIndex
					>> out.submeshes[i].numVertices
					>> out.submeshes[i].numIndices;
			}
			else {
				ar >> out.submeshes[i].startVertex
					>> out.submeshes[i].startIndex
					>> out.submeshes[i].numVertices
					>> util::read16(out.submeshes[i].numIndices);
			}
			out.submeshes[i].primitiveType = PrimitiveType::Triangle;
		}
	}
}


void MeshData::loadFromStream(std::istream &in_stream)
{
	auto ar = util::BinaryReader(in_stream);
	MeshDataHeader mdh;
	readMeshDataHeader(ar, mdh);
	assert(mdh.layout == 1 || mdh.layout == 2 || mdh.layout == 5);
	// assume single UV set
	uv.push_back(std::vector<glm::vec2>());
	auto &uv0 = uv[0];
	vertices.resize(mdh.num_vertices);
	normals.resize(mdh.num_vertices);
	tangents.resize(mdh.num_vertices);
	uv0.resize(mdh.num_vertices);	
	indices.resize(mdh.num_indices);
	submeshes = std::move(mdh.submeshes);

	for (auto i = 0u; i < mdh.num_vertices; ++i) 
	{
		auto &v = vertices[i];
		auto &n = normals[i];
		auto &t = tangents[i];
		auto &tx = uv0[i];
		float dummy;
		if (mdh.layout == 1 || mdh.layout == 2)
		{
			ar >> v.x >> v.y >> v.z
				>> n.x >> n.y >> n.z
				>> t.x >> t.y >> t.z
				>> dummy >> dummy >> dummy
				>> tx.x >> tx.y;
		}
		else if (mdh.layout == 5)
		{
			ar >> v.x >> v.y >> v.z
				>> n.x >> n.y >> n.z
				>> t.x >> t.y >> t.z
				>> tx.x >> tx.y;
		}
		if (mdh.layout == 2) {
			// skip 		
			glm::uint32 bone_ids;
			glm::vec4 bone_weights;
			ar >> bone_ids
				>> bone_weights.x 
				>> bone_weights.y 
				>> bone_weights.z 
				>> bone_weights.w;
		}
	}

	for (auto i = 0u; i < mdh.num_indices; ++i) 
	{
		ar >> util::read16(indices[i]);
	}
}