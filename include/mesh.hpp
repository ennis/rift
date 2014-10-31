#ifndef MESH_HPP
#define MESH_HPP

#include <resource.hpp>
#include <renderer.hpp>


// submesh description
struct SubMeshDesc {
	SubMeshDesc() = default;

	SubMeshDesc(
		PrimitiveType primitiveType, 
		int vertexStartOffset,
		int numVertices,
		int indexStartOffset,
		int numIndices) :
	mPrimitiveType(primitiveType),
	mVertexStartOffset(vertexStartOffset),
	mNumVertices(numVertices),
	mIndexStartOffset(indexStartOffset),
	mNumIndices(numIndices)
	{}

	PrimitiveType mPrimitiveType = PrimitiveType::Triangle;
	// offset in number of vertices
	int mVertexStartOffset = 0;
	int mNumVertices = -1;
	// offset in number of indices
	int mIndexStartOffset = 0;
	int mNumIndices = -1;
};

struct MeshVertex
{
	glm::vec3 position;
	glm::uint32 normal; // packSnorm3x10_1x2
	glm::uint32 tangent; // packSnorm3x10_1x2
	glm::uint32 bitangent;	// packSnorm3x10_1x2
	glm::uint32 texcoord;	// packUnorm2x16
};

//
// A mesh contains one or more buffers that store the vertex and (optionally) index data
// They have one or more submeshes (parts that have the same material)
// The buffers are not accessible by the user, and are owned by the Mesh object.
//
struct Mesh : public Resource
{
	void addSubMesh(SubMeshDesc const &desc);

	int getNumSubMeshes() const {
		return int(mSubMeshes.size());
	}

	SubMeshDesc const &getSubMesh(int index) const {
		assert(index < getNumSubMeshes());
		return mSubMeshes[index];
	}

	int getNumVertices() const {
		return mVertexBuffer->getNumVertices();
	}

	int getNumIndices() const {
		return mIndexBuffer->getNumIndices();
	}

	VertexBuffer *getVertexBuffer() {
		return mVertexBuffer;
	}

	IndexBuffer *getIndexBuffer() {
		return mIndexBuffer;
	}

	/*void updateVertex(
		int begin,
		int numVertices,
		MeshVertex const *data);

	void updateIndex(
		int begin,
		int numIndices,
		uint16_t const *data);*/

	static Mesh *create(
		Renderer &renderer,
		int numVertices, MeshVertex const *vertexData, 
		int numIndices, uint16_t const *indexData, 
		int numSubMeshes, SubMeshDesc const *subMeshData,
		ResourceUsage vertexBufferUsage = ResourceUsage::Static,
		ResourceUsage indexBufferUsage = ResourceUsage::Static);

	static Mesh *loadFromFile(
		Renderer &renderer,
		const char *path);

	// TODO: Mesh::initSharedResources (called from main init)
	static void initSharedResources(Renderer &renderer);

	static VertexLayout *getSharedVertexLayout();

protected:
	Mesh(VertexBuffer *vb, IndexBuffer *ib);

	virtual ~Mesh();

	// unique ptr
	VertexBuffer *mVertexBuffer;
	// unique ptr
	IndexBuffer *mIndexBuffer;
	std::vector<SubMeshDesc> mSubMeshes;

	// move this in mesh manager class
	// unique ptr
	static VertexLayout *sVertexLayout;
};



#endif