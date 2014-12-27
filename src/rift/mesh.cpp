#include <mesh.hpp>

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
	const void *initialIndices) 
{
	allocate(
		renderer,
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

Mesh::Mesh(Mesh &&rhs)
{
	*this = std::move(rhs);
}

Mesh::~Mesh()
{
	// TODO destroy
}

Mesh &Mesh::operator=(Mesh &&rhs)
{
	mRenderer = std::move(rhs.mRenderer);
	mVertexBuffers = std::move(rhs.mVertexBuffers);
	mIndexBuffer = std::move(rhs.mIndexBuffer);
	mVertexLayout = std::move(rhs.mVertexLayout);
	mPrimitiveType = std::move(rhs.mPrimitiveType);
	mStride = std::move(rhs.mStride);
	mIndexStride = std::move(rhs.mIndexStride);
	mNumBuffers = std::move(rhs.mNumBuffers);
	mNumVertices = std::move(rhs.mNumVertices);
	mNumIndices = std::move(rhs.mNumIndices);
	return *this;
}

void Mesh::allocate(
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
	const void *initialIndices)
{
	mRenderer = &renderer;
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
	mVertexLayout = renderer.createVertexLayout(numAttributes, elements);
	for (unsigned int ib = 0; ib < numBuffers; ++ib) {
		mVertexBuffers[ib] = renderer.createVertexBuffer(
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
		mIndexBuffer = renderer.createIndexBuffer(mIndexStride, numIndices, indexUsage, initialIndices);
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

void Mesh::draw() const
{
	if (mNumIndices == 0) {
		drawPart(0, mNumVertices);
	}
	else {
		drawPart(0, 0, mNumIndices);
	}
}

void Mesh::prepareDraw() const
{
	mRenderer->setVertexLayout(mVertexLayout);
	for (unsigned int ib = 0; ib < mNumBuffers; ++ib) {
		mRenderer->setVertexBuffer(ib, mVertexBuffers[ib]);
	}
}

void Mesh::drawPart(
	unsigned int baseVertex,
	unsigned int numVertices) const
{
	prepareDraw();
	mRenderer->draw(mPrimitiveType, baseVertex, numVertices);
}

void Mesh::drawPart(
	unsigned int baseVertex,
	unsigned int startIndex,
	unsigned int numIndices) const
{
	assert(mNumIndices != 0);
	prepareDraw();
	mRenderer->setIndexBuffer(mIndexBuffer);
	mRenderer->drawIndexed(
		mPrimitiveType, 
		baseVertex,
		0, 
		startIndex, 
		numIndices);
}
