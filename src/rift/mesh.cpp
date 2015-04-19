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

	struct MeshDataHeader
	{
		uint8_t version;
		uint8_t layout;
		unsigned num_vertices;
		unsigned num_indices;
		std::vector<Submesh> submeshes;
	};

	void readMeshDataHeader(
		serialization::IArchive &ar, 
		MeshDataHeader &out)
	{
		using namespace serialization;
		unsigned num_submeshes;
		ar >> read8(out.version)
			>> read8(out.layout)
			>> read16(num_submeshes)
			>> out.num_vertices
			>> out.num_indices;

		assert(out.version == 3);
		assert((out.layout == 1) || (out.layout == 2));
		assert(num_submeshes < 65536);
		assert(out.num_vertices < 40 * 1024 * 1024);
		assert(out.num_indices < 40 * 1024 * 1024);

		out.submeshes.resize(num_submeshes);
		for (auto i = 0u; i < num_submeshes; ++i) {
			ar >> out.submeshes[i].startVertex
				>> out.submeshes[i].startIndex
				>> out.submeshes[i].numVertices
				>> read16(out.submeshes[i].numIndices);
			out.submeshes[i].primitiveType = PrimitiveType::Triangle;
		}
	}
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
	MeshDataHeader mdh;
	readMeshDataHeader(ar, mdh);
	assert(mdh.layout == 1);

	// Layout type 1: not skinned
	std::vector<VertexType1> vs(mdh.num_vertices);
	std::vector<uint16_t> is(mdh.num_indices);
	for (auto i = 0u; i < mdh.num_vertices; ++i) {
		auto &v = vs[i];
		ar >> v.pos.x >> v.pos.y >> v.pos.z
			>> v.norm.x >> v.norm.y >> v.norm.z
			>> v.tg.x >> v.tg.y >> v.tg.z
			>> v.bitg.x >> v.bitg.y >> v.bitg.z
			>> v.tex.x >> v.tex.y;
	}
	for (auto i = 0u; i < mdh.num_indices; ++i) {
		ar >> read16(is[i]);
	}

	auto ptr = Mesh::create(
		{
			Attribute{ ElementFormat::Float3 },
			Attribute{ ElementFormat::Float3 },
			Attribute{ ElementFormat::Float3 },
			Attribute{ ElementFormat::Float3 },
			Attribute{ ElementFormat::Float2 }
		},
		mdh.num_vertices,
		vs.data(),
		mdh.num_indices,
		is.data(),
		mdh.submeshes);

	return ptr;
}

void Mesh::draw(RenderQueue &renderQueue, unsigned submesh)
{
	drawInstanced(renderQueue, submesh, 0, 1);
}

void Mesh::drawInstanced(RenderQueue &renderQueue, unsigned submesh, unsigned baseInstance, unsigned numInstances)
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
			baseInstance, numInstances);
	}
	else
	{
		renderQueue.draw(
			sm.primitiveType,
			sm.startVertex,
			sm.numVertices,
			baseInstance, numInstances);
	}
}

void SkinnedMesh::update(util::array_ref<glm::mat4> pose)
{
	auto nv = base_pos.size();

	auto ptr = dynamic_attribs->reserve_many<DynamicVertex>(nv);
	
	for (unsigned iv = 0; iv < nv; ++iv) 
	{
		auto const &v = base_pos[iv];
		auto &vout = ptr[iv];
		auto indices = bone_ids[iv];
		auto weights = bone_weights[iv];
		if (indices[0] == -1) continue;	// ???
		// bwaaaaah
		auto tf =
			pose[indices[0]] * weights[0] +
			pose[indices[1]] * weights[1] +
			pose[indices[2]] * weights[2] +
			pose[indices[3]] * weights[3];
		vout.pos = glm::vec3(tf * glm::vec4(v, 1.0f));
		vout.norm = base_norm[iv];
		vout.tg = base_tg[iv];
		vout.bitg = base_bitg[iv];
	}
}

SkinnedMesh::Ptr SkinnedMesh::loadFromArchive(serialization::IArchive &ar)
{
	using namespace serialization;
	auto pmesh = std::make_unique<SkinnedMesh>();
	auto &mesh = *pmesh;
	MeshDataHeader mdh;
	readMeshDataHeader(ar, mdh);

	assert(mdh.layout == 2);
	mesh.nbvertex = mdh.num_vertices;
	mesh.nbindex = mdh.num_indices;

	mesh.static_attribs = Buffer::create(
		sizeof(StaticVertex)*mdh.num_vertices,
		ResourceUsage::Static, 
		BufferUsage::VertexBuffer, 
		nullptr);

	mesh.ibo = Buffer::create(
		2 * mdh.num_indices,
		ResourceUsage::Static,
		BufferUsage::IndexBuffer, 
		nullptr);

	mesh.dynamic_attribs = Stream::create(
		BufferUsage::VertexBuffer, 
		sizeof(DynamicVertex)*mdh.num_vertices,
		3);

	auto ptr = mesh.static_attribs->map_as<StaticVertex>();
	mesh.base_pos.resize(mdh.num_vertices);
	mesh.base_norm.resize(mdh.num_vertices);
	mesh.base_tg.resize(mdh.num_vertices);
	mesh.base_bitg.resize(mdh.num_vertices);
	mesh.bone_ids.resize(mdh.num_vertices);
	mesh.bone_weights.resize(mdh.num_vertices);
	auto iptr = mesh.ibo->map_as<uint16_t>();

	for (auto i = 0u; i < mdh.num_vertices; ++i)
	{
		auto &pp = mesh.base_pos[i];
		auto &pn = mesh.base_norm[i];
		auto &ptg = mesh.base_tg[i];
		auto &pbitg = mesh.base_bitg[i];
		ar >> pp.x >> pp.y >> pp.z
			>> pn.x >> pn.y >> pn.z
			>> ptg.x >> ptg.y >> ptg.z
			>> pbitg.x >> pbitg.y >> pbitg.z
			>> ptr[i].tex.x >> ptr[i].tex.y
			>> mesh.bone_ids[i].x 
			>> mesh.bone_ids[i].y
			>> mesh.bone_ids[i].z
			>> mesh.bone_ids[i].w
			>> mesh.bone_weights[i].x
			>> mesh.bone_weights[i].y
			>> mesh.bone_weights[i].z
			>> mesh.bone_weights[i].w;
	}
	for (auto i = 0u; i < mdh.num_indices; ++i) {
		ar >> read16(iptr[i]);
	}

	mesh.layout = InputLayout::create(2, 
		{
			Attribute{ ElementFormat::Float3, 0 },
			Attribute{ ElementFormat::Float3, 0 },
			Attribute{ ElementFormat::Float3, 0 },
			Attribute{ ElementFormat::Float3, 0 },
			Attribute{ ElementFormat::Float2, 1 },
			Attribute{ ElementFormat::Uint8x4, 1 },
			Attribute{ ElementFormat::Float4, 1 }
		});

	return pmesh;
}

void SkinnedMesh::draw(
	RenderQueue &renderQueue, 
	unsigned submesh)
{
	drawInstanced(renderQueue, submesh, 0, 1);
}

void SkinnedMesh::drawInstanced(
	RenderQueue &renderQueue, 
	unsigned submesh,
	unsigned baseInstance, 
	unsigned numInstances)
{
	renderQueue.setVertexBuffers(
		{ 
			dynamic_attribs->getDescriptor(), 
			static_attribs->getDescriptor() 
		}, 
		*layout);
	// TODO factor out?
	const auto &sm = submeshes[submesh];
	if (nbindex)
	{
		renderQueue.setIndexBuffer(ibo->getDescriptor());
		renderQueue.drawIndexed(
			sm.primitiveType,
			sm.startIndex,
			sm.numIndices,
			sm.startVertex,
			baseInstance, 
			numInstances);
	}
	else
	{
		renderQueue.draw(
			sm.primitiveType,
			sm.startVertex,
			sm.numVertices,
			baseInstance, 
			numInstances);
	}
	dynamic_attribs->fence(renderQueue);
}