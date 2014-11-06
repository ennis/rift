#include <mesh.hpp>
#include <renderer.hpp>

//======================================
// MDAT
static const int MDAT_MaxSubMeshNameSize = 16;

enum MDAT_LayoutType
{
	MDAT_Layout_FullPacked = 1
};

// submesh descriptor
struct MDAT_SubMeshDesc
{
	uint32_t vertexOffset;
	uint32_t indexOffset;
	uint32_t numVertices;
	uint32_t numIndices;
	char name[MDAT_MaxSubMeshNameSize];
};

// file header
struct MDAT_Header
{
	char sig[4];	// 'MESH'
	uint16_t numSubMeshes;
	uint8_t vertexFormat;
	uint32_t dataOffset;
	uint32_t vertexSize;
	uint32_t indexSize;
};

//=============================================================================
//
void Mesh::addSubMesh(SubMeshDesc const &subMeshDesc)
{
	mSubMeshes.push_back(subMeshDesc);
}

//=============================================================================
// ctor
Mesh::Mesh(VertexBuffer *vb, IndexBuffer *ib) : 
mVertexBuffer(vb), 
mIndexBuffer(ib)
{}

//=============================================================================
// Destructor
Mesh::~Mesh() {
	mVertexBuffer->release();
	mIndexBuffer->release();
}

//=============================================================================
// create
Mesh *Mesh::create(
	Renderer &renderer,
	int numVertices, MeshVertex const *vertexData, 
	int numIndices, uint16_t const *indexData, 
	int numSubMeshes, SubMeshDesc const *subMeshData,
	ResourceUsage vertexBufferUsage,
	ResourceUsage indexBufferUsage)
{
	VertexBuffer *vb = renderer.createVertexBuffer(
		sizeof(MeshVertex), 
		numVertices, 
		vertexBufferUsage, 
		vertexData);
	// TODO 32-bit indices
	IndexBuffer *ib = renderer.createIndexBuffer(
		sizeof(uint16_t), 
		numIndices, 
		indexBufferUsage, 
		indexData);
	Mesh *mesh = new Mesh(vb, ib);	
	return mesh;
}

//=============================================================================
// loadFromFile
// TODO rewrite with endianness-safe functions
Mesh *Mesh::loadFromFile(
	Renderer &renderer, 
	const char *path)
{
	MDAT_Header header;
	std::ifstream fileIn(path, std::ios::in | std::ios::binary);
	assert(fileIn.is_open());
	fileIn.read(reinterpret_cast<char*>(&header), sizeof(MDAT_Header));
	assert(header.sig[0] == 'M' && 
		header.sig[1] == 'E' &&
		header.sig[2] == 'S' &&
		header.sig[3] == 'H');
	std::size_t vertexBufferSize;
	std::size_t indexBufferSize;
	if (header.vertexFormat != MDAT_Layout_FullPacked) {
		ERROR << path << ": unrecognized vertex format (" << header.vertexFormat << ")";
		throw std::runtime_error("MDAT: unrecognized vertex format");
	}
	vertexBufferSize = header.vertexSize * sizeof(MeshVertex);
	indexBufferSize = header.indexSize * 2;
	MDAT_SubMeshDesc *subMeshDesc = new MDAT_SubMeshDesc[header.numSubMeshes];
	fileIn.read(
		reinterpret_cast<char*>(subMeshDesc), 
		header.numSubMeshes * sizeof(MDAT_SubMeshDesc));
	char *vertexData = new char[vertexBufferSize];
	char *indexData = nullptr;
	if (indexBufferSize) {
		indexData = new char[indexBufferSize];
	}
	// vertex data follows in the file
	fileIn.read(vertexData, vertexBufferSize);
	if (indexBufferSize) {
		fileIn.read(indexData, indexBufferSize);
	}
	Mesh *mesh = Mesh::create(
		renderer, 
		header.vertexSize,
		reinterpret_cast<MeshVertex const*>(vertexData), 
		header.indexSize, 
		reinterpret_cast<uint16_t const*>(indexData), 
		0, 
		nullptr);
	mesh->mSubMeshes.reserve(header.numSubMeshes);
	for (int i = 0; i < header.numSubMeshes; ++i) {
		SubMeshDesc smd;
		smd.mNumIndices = subMeshDesc[i].numIndices;
		smd.mNumVertices = subMeshDesc[i].numVertices;
		smd.mVertexStartOffset = subMeshDesc[i].vertexOffset;
		smd.mIndexStartOffset = subMeshDesc[i].indexOffset;
		// TODO support for other primitives in binary file
		smd.mPrimitiveType = PrimitiveType::Triangle;
		mesh->addSubMesh(smd);
	}
	delete[] vertexData;
	delete[] indexData;
	delete[] subMeshDesc;
	return mesh;
}

//=============================================================================
// TODO Put this somewhere else?
VertexLayout *Mesh::sVertexLayout = nullptr;

//=============================================================================
void Mesh::initSharedResources(Renderer &renderer)
{
	static const int kMeshVertexPositionOffset = offsetof(MeshVertex, position);
	static const int kMeshVertexNormalOffset = offsetof(MeshVertex, normal);
	static const int kMeshVertexTangentOffset = offsetof(MeshVertex, tangent);
	static const int kMeshVertexBitangentOffset = offsetof(MeshVertex, bitangent);
	static const int kMeshVertexTexcoordOffset = offsetof(MeshVertex, texcoord);
	static const int kMeshVertexStride = sizeof(MeshVertex);

	VertexElement vertexElements[5] = {
		VertexElement(0, 0, kMeshVertexPositionOffset, kMeshVertexStride, ElementFormat::Float3),
		VertexElement(1, 0, kMeshVertexNormalOffset, kMeshVertexStride, ElementFormat::Snorm10x3_1x2),
		VertexElement(2, 0, kMeshVertexTangentOffset, kMeshVertexStride, ElementFormat::Snorm10x3_1x2),
		VertexElement(3, 0, kMeshVertexBitangentOffset, kMeshVertexStride, ElementFormat::Snorm10x3_1x2),
		VertexElement(4, 0, kMeshVertexTexcoordOffset, kMeshVertexStride, ElementFormat::Unorm16x2),
	};

	sVertexLayout = renderer.createVertexLayout(5, vertexElements);
}

//=============================================================================
VertexLayout *Mesh::getSharedVertexLayout()
{
	return sVertexLayout;
}