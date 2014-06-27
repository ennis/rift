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
		outData[i].position.x = init.positions[3*i];
		outData[i].position.y = init.positions[3*i+1];
		outData[i].position.z = init.positions[3*i+2];
	}
	if (init.normals) {
		for (int i = 0; i < init.desc.numVertices; ++i) {
			outData[i].normal.x = init.normals[3 * i];
			outData[i].normal.y = init.normals[3 * i + 1];
			outData[i].normal.z = init.normals[3 * i + 2];
		}
	}
	if (init.texcoords) {
		for (int i = 0; i < init.desc.numVertices; ++i) {
			outData[i].texcoord.x = init.texcoords[2*i];
			outData[i].texcoord.y = init.texcoords[2*i+1];
		}
	}
	if (init.colors) {
		for (int i = 0; i < init.desc.numVertices; ++i) {
			outData[i].color.r = init.colors[4*i];
			outData[i].color.g = init.colors[4*i+1];
			outData[i].color.b = init.colors[4*i+2];
			outData[i].color.a = init.colors[4*i+3];
		}
	}
}

CGL3MeshBuffer::~CGL3MeshBuffer()
{
}

void CGL3MeshBuffer::destroy()
{
	GLCHECK(glDeleteVertexArrays(1, &mVAO));
	GLCHECK(glDeleteBuffers(1, &mVBO));
	GLCHECK(glDeleteBuffers(1, &mIBO));
	delete this;
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
	// offsets
	static const unsigned int GL3_LayoutFull_PositionOffset = 0;
	static const unsigned int GL3_LayoutFull_NormalOffset = 3*4;
	static const unsigned int GL3_LayoutFull_TexcoordOffset = 6*4;
	static const unsigned int GL3_LayoutFull_ColorOffset = 8*4;
	static const unsigned int GL3_LayoutFull_Stride = 12*4;

	// create VAO
	GLCHECK(glGenVertexArrays(1, &mVAO));
	GLCHECK(glBindVertexArray(mVAO));
	GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, mVBO));

	// full layout
	GLCHECK(glEnableVertexAttribArray(GL3_Attrib_Position));
	GLCHECK(glVertexAttribPointer(GL3_Attrib_Position, 3, GL_FLOAT, GL_FALSE, GL3_LayoutFull_Stride, reinterpret_cast<void*>(GL3_LayoutFull_PositionOffset)));
	
	GLCHECK(glEnableVertexAttribArray(GL3_Attrib_Normal));
	GLCHECK(glVertexAttribPointer(GL3_Attrib_Normal, 3, GL_FLOAT, GL_FALSE, GL3_LayoutFull_Stride, reinterpret_cast<void*>(GL3_LayoutFull_NormalOffset)));

	GLCHECK(glEnableVertexAttribArray(GL3_Attrib_Texcoord));
	GLCHECK(glVertexAttribPointer(GL3_Attrib_Texcoord, 2, GL_FLOAT, GL_FALSE, GL3_LayoutFull_Stride, reinterpret_cast<void*>(GL3_LayoutFull_TexcoordOffset)));

	GLCHECK(glEnableVertexAttribArray(GL3_Attrib_Color));
	GLCHECK(glVertexAttribPointer(GL3_Attrib_Color, 4, GL_FLOAT, GL_FALSE, GL3_LayoutFull_Stride, reinterpret_cast<void*>(GL3_LayoutFull_ColorOffset)));

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