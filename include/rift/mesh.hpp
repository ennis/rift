#ifndef Mesh_HPP
#define Mesh_HPP

#include <renderer.hpp>

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

	Mesh(Renderer &renderer);
	Mesh(Renderer &renderer, const char *filePath);
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

	~Mesh();

	void allocate(
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

	Renderer *mRenderer;	// borrowed ref
	// TODO usage-agnostic buffer class
	VertexBuffer *mVertexBuffers[16];	// owned
	IndexBuffer *mIndexBuffer;	// owned
	VertexLayout *mVertexLayout;	// owned
	PrimitiveType mPrimitiveType;
	unsigned int mStride[16];
	unsigned int mIndexStride;
	unsigned int mNumBuffers;
	unsigned int mNumVertices;
	unsigned int mNumIndices; 
};

 
#endif /* end of include guard: Mesh_HPP */