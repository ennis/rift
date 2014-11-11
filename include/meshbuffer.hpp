#ifndef MESHBUFFER_HPP
#define MESHBUFFER_HPP

#include <renderer.hpp>

class MeshBuffer
{
public:
	MeshBuffer(Renderer &renderer);
	MeshBuffer(Renderer &renderer, const char *filePath);
	MeshBuffer(
		Renderer &renderer,
		PrimitiveType primitiveType,
		int numElements,
		ElementFormat *layoutDesc,
		ResourceUsage resourceUsage,
		int numVertices,
		const void *initialVertices,
		int numIndices = 0,
		const void *initialIndices = nullptr);

	~MeshBuffer();

	void allocate(
		PrimitiveType primitiveType,
		int numElements, 
		ElementFormat *layoutDesc, 
		ResourceUsage resourceUsage,
		int numVertices, 
		const void *initialVertices,
		int numIndices = 0,
		const void *initialIndices = nullptr);

	void update(
		unsigned int startVertex,
		unsigned int numVertices, 
		const void *data);

	void updateIndices(
		unsigned int startIndex, 
		unsigned int numVertices,
		const void *data);

	VertexBuffer *getVertexBuffer();
	IndexBuffer *getIndexBuffer();
	VertexLayout *getVertexLayout();

	void draw();
	void drawPart(
		unsigned int startIndex,	// or startVertex if no indexBuffer 
		unsigned int numVertices);

private:
	Renderer *mRenderer;	// borrowed ref
	// TODO usage-agnostic buffer class
	VertexBuffer *mVertexBuffer;	// owned
	IndexBuffer *mIndexBuffer;	// owned
	VertexLayout *mVertexLayout;	// owned
	PrimitiveType mPrimitiveType;
	unsigned int mStride;
	unsigned int mNumVertices;
	unsigned int mNumIndices; 
};

 
#endif /* end of include guard: MESHBUFFER_HPP */