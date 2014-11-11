#include <meshbuffer.hpp>

MeshBuffer::MeshBuffer(Renderer &renderer) : mRenderer(&renderer)
{}

MeshBuffer::MeshBuffer(Renderer &renderer, const char *filePath) : mRenderer(&renderer)
{
	// TODO load from file
}

MeshBuffer::MeshBuffer(
	Renderer &renderer,
	PrimitiveType primitiveType, 
	int numElements, 
	ElementFormat *layoutDesc,
	ResourceUsage resourceUsage, 
	int numVertices, 
	const void *initialVertices, 
	int numIndices, 
	const void *initialIndices) :
mRenderer(&renderer)
{
	allocate(
		primitiveType, 
		numElements, 
		layoutDesc, 
		resourceUsage,
		numVertices, 
		initialVertices,
		numIndices, 
		initialIndices);
}

MeshBuffer::~MeshBuffer()
{
	// TODO destroy
}

void MeshBuffer::allocate(
	PrimitiveType primitiveType,
	int numElements, 
	ElementFormat *layoutDesc, 
	ResourceUsage resourceUsage,
	int numVertices, 
	const void *initialVertices,
	int numIndices,
	const void *initialIndices)
{
	mPrimitiveType = primitiveType;
	mNumVertices = numVertices;
	mNumIndices = numIndices;
	// vertex layout
	VertexElement elements[VertexLayout::kNumVertexElements];
	unsigned int offset = 0;
	for (int i = 0; i < numElements; ++i) {
		auto &e = elements[i];
		e.inputSlot = i;
		e.bufferSlot = 0;
		e.offset = offset;
		e.format = layoutDesc[i];
		offset += getElementFormatSize(e.format);
	}
	mStride = offset;
	for (int i = 0; i < numElements; ++i) {
		auto &e = elements[i];
		e.stride = mStride;
	}
	mVertexLayout = mRenderer->createVertexLayout(numElements, elements);
	mVertexBuffer = mRenderer->createVertexBuffer(mStride, numVertices, resourceUsage, initialVertices);
	mIndexBuffer = nullptr;
	if (numIndices != 0) {
		mIndexBuffer = mRenderer->createIndexBuffer(2, numIndices, resourceUsage, initialIndices);
	}
}

void MeshBuffer::update(
	unsigned int startVertex,
	unsigned int numVertices, 
	const void *data)
{
	mRenderer->updateBuffer(mVertexBuffer,startVertex*mStride,numVertices*mStride,data);
}

void MeshBuffer::updateIndices(
	unsigned int startIndex, 
	unsigned int numIndices,
	const void *data)
{
	mRenderer->updateBuffer(mIndexBuffer,startIndex*2,numIndices*2,data);
}

VertexBuffer *MeshBuffer::getVertexBuffer() 
{
	return mVertexBuffer;
}

IndexBuffer *MeshBuffer::getIndexBuffer()
{
	return mIndexBuffer;
}

VertexLayout *MeshBuffer::getVertexLayout()
{
	return mVertexLayout;
}

void MeshBuffer::draw()
{
	drawPart(0, (mNumIndices == 0) ? mNumVertices : mNumIndices);
}

void MeshBuffer::drawPart(
	unsigned int startIndex,	// or startVertex if no indexBuffer 
	unsigned int numVertices)
{
	mRenderer->setVertexLayout(mVertexLayout);
	mRenderer->setVertexBuffer(0, mVertexBuffer);
	if (mNumIndices != 0) {
		mRenderer->setIndexBuffer(mIndexBuffer);
		mRenderer->drawIndexed(
			mPrimitiveType, 
			0,
			mNumVertices, 
			startIndex, 
			numVertices);
	}
	else {
		mRenderer->draw(mPrimitiveType, startIndex, numVertices);
	}
}
