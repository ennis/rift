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
	// TODO factor out common loading code
	using namespace serialization;
	auto pmesh = std::make_unique<SkinnedMesh>();
	auto &mesh = *pmesh;

	unsigned int num_submeshes, layout, version;

	ar >> read8(version)
		>> read8(layout)
		>> read16(num_submeshes)
		>> mesh.nbvertex
		>> mesh.nbindex;

	assert(version == 3);
	assert((layout == 1) || (layout == 2));
	assert(num_submeshes < 65536);
	assert(mesh.nbvertex < 40 * 1024 * 1024);
	assert(mesh.nbindex < 40 * 1024 * 1024);

	mesh.submeshes.resize(num_submeshes);
	for (auto i = 0u; i < num_submeshes; ++i) {
		ar >> mesh.submeshes[i].startVertex
			>> mesh.submeshes[i].startIndex
			>> mesh.submeshes[i].numVertices
			>> read16(mesh.submeshes[i].numIndices);
		mesh.submeshes[i].primitiveType = PrimitiveType::Triangle;
	}

	if (layout == 1)
	{
		// OOPS
		return nullptr;
	}
	else
	{
		mesh.static_attribs = Buffer::create(
			sizeof(StaticVertex)*mesh.nbvertex, 
			ResourceUsage::Static, 
			BufferUsage::VertexBuffer, 
			nullptr);

		mesh.ibo = Buffer::create(
			2 * mesh.nbindex, 
			ResourceUsage::Static,
			BufferUsage::IndexBuffer, 
			nullptr);

		mesh.dynamic_attribs = Stream::create(
			BufferUsage::VertexBuffer, 
			sizeof(DynamicVertex)*mesh.nbvertex, 
			3);

		auto ptr = mesh.static_attribs->map_as<StaticVertex>();
		mesh.base_pos.resize(mesh.nbvertex);
		mesh.base_norm.resize(mesh.nbvertex);
		mesh.base_tg.resize(mesh.nbvertex);
		mesh.base_bitg.resize(mesh.nbvertex);
		mesh.bone_ids.resize(mesh.nbvertex);
		mesh.bone_weights.resize(mesh.nbvertex);
		auto iptr = mesh.ibo->map_as<uint16_t>();

		for (auto i = 0u; i < mesh.nbvertex; ++i)
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
		for (auto i = 0u; i < mesh.nbindex; ++i) {
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