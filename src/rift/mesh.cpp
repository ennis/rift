#include <mesh.hpp>

Mesh::Mesh(Renderer &renderer) : mRenderer(&renderer)
{}

Mesh::Mesh(Renderer &renderer, const char *filePath) : mRenderer(&renderer)
{
	// TODO load from file
}

Mesh::Mesh(
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
	const void *initialIndices) :
mRenderer(&renderer)
{
	allocate(
		primitiveType, 
		numAttributes,
		attributes, 
		numBuffers,
		buffers,
		numVertices, 
		initialVertices,
		numIndices, 
		indexFormat,
		indexUsage,
		initialIndices);
}

Mesh::~Mesh()
{
	// TODO destroy
}

void Mesh::allocate(
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
	const void *initialIndices)
{
	mPrimitiveType = primitiveType;
	mNumVertices = numVertices;
	mNumIndices = numIndices;
	mNumBuffers = numBuffers;
	// vertex layout
	VertexElement elements[VertexLayout::kNumVertexElements];
	for (unsigned int i = 0; i < 16; ++i) mStride[i] = 0;
	for (unsigned int i = 0; i < numAttributes; ++i) {
		auto &e = elements[i];
		e.inputSlot = i;
		assert(attributes[i].buffer < numBuffers);
		e.bufferSlot = attributes[i].buffer;
		e.offset = mStride[e.bufferSlot];
		e.format = attributes[i].elementFormat;
		mStride[e.bufferSlot] += getElementFormatSize(e.format);
	}
	for (unsigned int i = 0; i < numAttributes; ++i) {
		auto &e = elements[i];
		e.stride = mStride[e.bufferSlot];
	}
	mVertexLayout = mRenderer->createVertexLayout(numAttributes, elements);
	for (unsigned int ib = 0; ib < numBuffers; ++ib) {
		mVertexBuffers[ib] = mRenderer->createVertexBuffer(
			mStride[ib], 
			numVertices, 
			buffers[ib].usage, 
			initialVertices ? initialVertices[ib] : nullptr);
	}
	for (unsigned int ib = numBuffers; ib < 16; ++ib) {
		mVertexBuffers[ib] = nullptr;
	}
	if (numIndices != 0) {
		mIndexStride = getElementFormatSize(indexFormat);
		mIndexBuffer = mRenderer->createIndexBuffer(mIndexStride, numIndices, indexUsage, initialIndices);
	}
	else {
		mIndexBuffer = nullptr;
		mIndexStride = 0;
	}
}

void Mesh::update(
	unsigned int buffer,
	unsigned int startVertex,
	unsigned int numVertices, 
	const void *data)
{
	mRenderer->updateBuffer(mVertexBuffers[buffer],startVertex*mStride[buffer],numVertices*mStride[buffer],data);
}

void Mesh::updateIndices(
	unsigned int startIndex, 
	unsigned int numIndices,
	const void *data)
{
	mRenderer->updateBuffer(mIndexBuffer, startIndex*mIndexStride, numIndices * mIndexStride, data);
}

VertexBuffer *Mesh::getVertexBuffer(unsigned int id) 
{
	return mVertexBuffers[id];
}

IndexBuffer *Mesh::getIndexBuffer()
{
	return mIndexBuffer;
}

VertexLayout *Mesh::getVertexLayout()
{
	return mVertexLayout;
}

void Mesh::draw()
{
	drawPart(0, (mNumIndices == 0) ? mNumVertices : mNumIndices);
}

void Mesh::drawPart(
	unsigned int startIndex,	// or startVertex if no indexBuffer 
	unsigned int numVertices)
{
	mRenderer->setVertexLayout(mVertexLayout);
	for (unsigned int ib = 0; ib < mNumBuffers; ++ib) {
		mRenderer->setVertexBuffer(ib, mVertexBuffers[ib]);
	}
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
