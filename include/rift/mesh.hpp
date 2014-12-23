#ifndef Mesh_HPP
#define Mesh_HPP

#include <renderer.hpp>
#include <array>

class Mesh
{
public:
	struct Attribute {
		unsigned int buffer;
		ElementFormat elementFormat;
	};

	struct Buffer {
		ResourceUsage usage;
	};

	Mesh() = default;
	Mesh(
		Renderer &renderer,
		PrimitiveType primitiveType,
		unsigned int numAttributes,
		Mesh::Attribute attributes[],
		unsigned int numBuffers,
		Mesh::Buffer buffers[],
		unsigned int numVertices,
		const void *initialVertices[],
		unsigned int numIndices,
		ElementFormat indexFormat,
		ResourceUsage indexUsage,
		const void *initialIndices);
	Mesh(Mesh const &rhs) = delete;
	Mesh(Mesh &&rhs);
	~Mesh();
	Mesh &operator=(Mesh &&rhs);

	void allocate(
		Renderer &renderer,
		PrimitiveType primitiveType,
		unsigned int numAttributes,
		Mesh::Attribute attributes[],
		unsigned int numBuffers,
		Mesh::Buffer buffers[],
		unsigned int numVertices,
		const void *initialVertices[],
		unsigned int numIndices,
		ElementFormat indexFormat,
		ResourceUsage indexUsage,
		const void *initialIndices);

	void update(
		unsigned int buffer,
		unsigned int startVertex,
		unsigned int numVertices, 
		const void *data);

	void updateIndices(
		unsigned int startIndex, 
		unsigned int numVertices,
		const void *data);

	VertexBuffer *getVertexBuffer(unsigned int id);
	IndexBuffer *getIndexBuffer();
	VertexLayout *getVertexLayout();

	void draw();
	void drawPart(
		unsigned int baseVertex,
		unsigned int numVertices);
	void drawPart(
		unsigned int baseVertex,
		unsigned int startIndex, 
		unsigned int numIndices);

	unsigned int getNumVertices() const {
		return mNumVertices;
	}
	unsigned int getNumIndices() const {
		return mNumIndices;
	}

private:

	void prepareDraw();

	Renderer *mRenderer = nullptr;	// borrowed ref
	// TODO usage-agnostic buffer class
	std::array<VertexBuffer*, 16> mVertexBuffers;	// owned
	IndexBuffer *mIndexBuffer = nullptr;	// owned
	VertexLayout *mVertexLayout = nullptr;	// owned
	PrimitiveType mPrimitiveType;
	std::array<unsigned int, 16> mStride;
	unsigned int mIndexStride = 0;
	unsigned int mNumBuffers = 0;
	unsigned int mNumVertices = 0;
	unsigned int mNumIndices = 0;
};

 
#endif /* end of include guard: Mesh_HPP */