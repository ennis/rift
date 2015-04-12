#include <mesh.hpp>

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
}

Mesh::Mesh(
	util::array_ref<Attribute> layout_,
	int numVertices,
	const void *vertexData,
	int numIndices,
	const void *indexData,
	util::array_ref<Submesh> submeshes_)
{
	layout = InputLayout::create(1, layout_);
	nbvertex = numVertices;
	stride = layout->strides[0];
	auto vbsize = stride*numVertices;
	nbindex = numIndices;
	auto ibsize = numIndices * 2;

	vbo = Buffer::create(vbsize, ResourceUsage::Static, BufferUsage::VertexBuffer, vertexData);
	if (numIndices)
		ibo = Buffer::create(ibsize, ResourceUsage::Static, BufferUsage::IndexBuffer, indexData);

	submeshes.assign(submeshes_.begin(), submeshes_.end());
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
	assert((layout == 1) || (layout == 2));
	assert(num_submeshes < 65536);
	assert(num_vertices < 40 * 1024 * 1024);
	assert(num_indices < 40 * 1024 * 1024);

	submeshes.resize(num_submeshes);
	for (auto i = 0u; i < num_submeshes; ++i) {
		ar >> submeshes[i].startVertex
			>> submeshes[i].startIndex
			>> submeshes[i].numVertices
			>> read16(submeshes[i].numIndices);
		submeshes[i].primitiveType = PrimitiveType::Triangle;
	}

	if (layout == 1)
	{
		// Layout type 1: not skinned
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
			{
				Attribute{ ElementFormat::Float3 },
				Attribute{ ElementFormat::Float3 },
				Attribute{ ElementFormat::Float3 },
				Attribute{ ElementFormat::Float3 },
				Attribute{ ElementFormat::Float2 }
			},
			num_vertices,
			vs.data(),
			num_indices,
			is.data(),
			submeshes);
		return ptr;
	}
	else
	{
		// Layout type 2: skinned mesh
		std::vector<VertexType2> vs(num_vertices);
		std::vector<uint16_t> is(num_indices);
		for (auto i = 0u; i < num_vertices; ++i) {
			auto &v = vs[i];
			ar >> v.pos.x >> v.pos.y >> v.pos.z
				>> v.norm.x >> v.norm.y >> v.norm.z
				>> v.tg.x >> v.tg.y >> v.tg.z
				>> v.bitg.x >> v.bitg.y >> v.bitg.z
				>> v.tex.x >> v.tex.y
				>> v.bone_ids
				>> v.bone_weights.x
				>> v.bone_weights.y
				>> v.bone_weights.z
				>> v.bone_weights.w;
		}
		for (auto i = 0u; i < num_indices; ++i) {
			ar >> read16(is[i]);
		}

		// create mesh
		auto ptr = Mesh::create(
			{
				Attribute{ ElementFormat::Float3 },
				Attribute{ ElementFormat::Float3 },
				Attribute{ ElementFormat::Float3 },
				Attribute{ ElementFormat::Float3 },
				Attribute{ ElementFormat::Float2 },
				Attribute{ ElementFormat::Uint8x4 },
				Attribute{ ElementFormat::Float4 }
			},
			num_vertices,
			vs.data(),
			num_indices,
			is.data(),
			submeshes);
		return ptr;
	}
}

void Mesh::draw(RenderQueue2 &renderQueue, unsigned submesh)
{
	renderQueue.setVertexBuffers({ vbo->getDescriptor() }, *layout);
	const auto &sm = submeshes[submesh];
	if (nbindex)
	{
		renderQueue.setIndexBuffer(ibo->getDescriptor());
		renderQueue.drawIndexed(
			sm.primitiveType,
			sm.startIndex,
			sm.numIndices,
			sm.startVertex,
			0, 1);
	}
	else
	{
		renderQueue.draw(
			sm.primitiveType,
			sm.startVertex,
			sm.numVertices,
			0, 1);
	}
	
}
