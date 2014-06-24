#include <gl3meshbuffer.hpp>
#include <gl3error.hpp>
#include <log.hpp>
#include <vector>

static GLenum primitiveTypeToGLenum(PrimitiveType type)
{
	switch (type)
	{
	case PrimitiveType::Lines:
		return GL_LINES;
	case PrimitiveType::Points:
		return GL_POINTS;
	//case PrimitiveType::Quads:
	//	return GL_QUADS;
	case PrimitiveType::Triangles:
		return GL_TRIANGLES;
	default:
		break;
	}

	return GL_POINTS;
}

static GLuint createBuffer(std::size_t size, GLenum usage)
{
	GLuint obj;
	GLCHECK(glGenBuffers(1, &obj));
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, obj));
	GLCHECK(glBufferData(GL_ARRAY_BUFFER, size, nullptr, usage));
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	return obj;
}

static void loadInterleavedMeshData(MeshBufferInit &init, std::vector<VertexLayoutFull> &outData)
{
	outData.resize(init.desc.numVertices);
	assert(init.positions);
	for (int i = 0; i < init.desc.numVertices; ++i) {
		outData[i].position = init.positions[i];
	}
	if (init.normals) {
		for (int i = 0; i < init.desc.numVertices; ++i) {
			outData[i].normal = init.positions[i];
		}
	}
	if (init.texcoords) {
		for (int i = 0; i < init.desc.numVertices; ++i) {
			outData[i].texcoord = init.texcoords[i];
		}
	}
	if (init.colors) {
		for (int i = 0; i < init.desc.numVertices; ++i) {
			outData[i].color = init.colors[i];
		}
	}
}

CGL3MeshBuffer::~CGL3MeshBuffer()
{
	deleteResource();
}

void CGL3MeshBuffer::deleteResource()
{
	GLCHECK(glDeleteVertexArrays(1, &mVAO));
	GLCHECK(glDeleteBuffers(1, &mVBO));
	GLCHECK(glDeleteBuffers(1, &mIBO));
}

void CGL3MeshBuffer::allocate(MeshBufferInit &init)
{
	mDesc = init.desc;
	// assume full layout
	mVBOSize = mDesc.numVertices*sizeof(VertexLayoutFull);
	mIBOSize = mDesc.numIndices*sizeof(uint16_t);
	mVBO = createBuffer(mVBOSize, GL_STATIC_DRAW);
	if (mDesc.numIndices > 0)
		mIBO = createBuffer(mIBOSize, GL_STATIC_DRAW);

	// convert data to target format
	std::vector<VertexLayoutFull> outData;
	loadInterleavedMeshData(init, outData);
	// send data to GPU
	updateVertex(outData.data(), 0, mVBOSize);
	if (mDesc.numIndices > 0)
		updateIndex(init.indices, 0, mIBOSize);
	
	LOG << "created mesh buffer vb " << mVBO << "(" << mVBOSize << ") ib " << mIBO << " (" << mIBOSize << ")";
	setupVAO();
}

void CGL3MeshBuffer::setupVAO()
{
	// create VAO
	GLCHECK(glGenVertexArrays(1, &mVAO));
	GLCHECK(glBindVertexArray(mVAO));
	// #0: position
	// TODO other attributes
	GLCHECK(glEnableVertexAttribArray(0));
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	GLCHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexLayoutFull), nullptr));
	GLCHECK(glBindVertexArray(0));
}

void CGL3MeshBuffer::updateVertex(void const *vertexData, std::size_t offset, std::size_t size)
{
	assert(mVBO != -1);
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	GLCHECK(glBufferSubData(GL_ARRAY_BUFFER, offset, size, vertexData));
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void CGL3MeshBuffer::updateIndex(void const *indexData, std::size_t offset, std::size_t size)
{
	assert(mIBO != -1);
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, mIBO));
	GLCHECK(glBufferSubData(GL_ARRAY_BUFFER, offset, size, indexData));
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void CGL3MeshBuffer::draw()
{
	assert(mVAO != -1);
	GLCHECK(glBindVertexArray(mVAO));
	if (mDesc.numIndices > 0) {
		GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
		GLCHECK(glDrawElements(primitiveTypeToGLenum(mDesc.primitiveType), mDesc.numIndices, GL_UNSIGNED_SHORT, nullptr));
		GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}
	else {
		GLCHECK(glDrawArrays(primitiveTypeToGLenum(mDesc.primitiveType), 0, mDesc.numVertices));
	}
	GLCHECK(glBindVertexArray(0));
}