#include <mesh.hpp>

Mesh::Mesh(
	PrimitiveType primitiveType,
	std::array_ref<Mesh::Attribute> attributes,
	std::array_ref<Mesh::BufferDesc> buffers,
	unsigned int numVertices,
	const void *initialVertices[],
	unsigned int numIndices,
	ElementFormat indexFormat,
	ResourceUsage indexUsage,
	const void *initialIndices) 
{
	allocate(
		primitiveType, 
		attributes, 
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
	mVertexBuffers = std::move(rhs.mVertexBuffers);
	mIndexBuffer = std::move(rhs.mIndexBuffer);
	mVertexLayout = std::move(rhs.mVertexLayout);
	mPrimitiveType = std::move(rhs.mPrimitiveType);
	mStrides = std::move(rhs.mStrides);
	mIndexStride = std::move(rhs.mIndexStride);
	mNumBuffers = std::move(rhs.mNumBuffers);
	mNumVertices = std::move(rhs.mNumVertices);
	mNumIndices = std::move(rhs.mNumIndices);
	return *this;
}

void Mesh::allocate(
	PrimitiveType primitiveType,
	std::array_ref<Mesh::Attribute> attributes,
	std::array_ref<Mesh::BufferDesc> buffers,
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
	mNumBuffers = buffers.size();
	// vertex layout
	VertexElement2 elements[16];
	for (int i = 0; i < 16; ++i) mStrides[i] = 0;
	for (int i = 0; i < attributes.size(); ++i) {
		auto &e = elements[i];
		assert(attributes[i].inputSlot < mNumBuffers);
		e.inputSlot = attributes[i].inputSlot;
		e.offset = mStrides[e.inputSlot];
		e.format = attributes[i].elementFormat;
		mStrides[e.inputSlot] += getElementFormatSize(e.format);
	}
	mVertexLayout = VertexLayout(std::make_array_ref<VertexElement2>(elements, attributes.size()));
	for (int ib = 0; ib < mNumBuffers; ++ib) {
		mVertexBuffers.push_back(
			Buffer(
				mStrides[ib] * mNumVertices, 
				buffers[ib].usage, 
				BufferUsage::VertexBuffer, 
				initialVertices ? initialVertices[ib] : nullptr));
	}
	if (numIndices != 0) {
		// allocate an index buffer
		mIndexStride = getElementFormatSize(indexFormat);
		mIndexBuffer = Buffer(
			numIndices * mIndexStride, 
			indexUsage, 
			BufferUsage::IndexBuffer, 
			initialIndices);
	}
	else {
		mIndexStride = 0;
	}
}

void Mesh::update(
	unsigned int buffer,
	unsigned int startVertex,
	unsigned int numVertices,
	const void *data)
{
	mVertexBuffers[buffer].update(startVertex*mStrides[buffer], numVertices*mStrides[buffer], data);
}

void Mesh::updateIndices(
	unsigned int indexOffset,
	unsigned int numIndices,
	const void *data)
{
	mIndexBuffer.update(indexOffset * mIndexStride, numIndices * mIndexStride, data);
}

void Mesh::draw(Renderer &renderer) const
{
	if (mNumIndices == 0) {
		drawPart(renderer, 0, mNumVertices);
	}
	else {
		drawPart(renderer, 0, 0, mNumIndices);
	}
}

void Mesh::prepareDraw(Renderer &renderer) const
{
	renderer.setInputLayout(&mVertexLayout);
	for (int ib = 0; ib < mNumBuffers; ++ib) {
		renderer.setVertexBuffer(ib, &mVertexBuffers[ib], 0, mStrides[ib]);
	}
}

void Mesh::drawPart(
	Renderer &renderer,
	unsigned int baseVertex,
	unsigned int numVertices) const
{
	prepareDraw(renderer);
	renderer.draw(mPrimitiveType, baseVertex, numVertices);
}

void Mesh::drawPart(
	Renderer &renderer,
	unsigned int baseVertex,
	unsigned int indexOffset,
	unsigned int numIndices) const
{
	assert(mNumIndices != 0);
	prepareDraw(renderer);
	renderer.setIndexBuffer(&mIndexBuffer, ElementFormat::Uint16);
	renderer.drawIndexed(
		mPrimitiveType,
		indexOffset,
		numIndices,
		baseVertex);
}
