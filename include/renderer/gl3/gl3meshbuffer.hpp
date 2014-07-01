#ifndef GL3MESHBUFFER_HPP
#define GL3MESHBUFFER_HPP

#include <common.hpp>
#include <meshbuffer.hpp>
#include <opengl.hpp>

// attribute locations
enum GL3Attribute
{
	GL3_Attrib_Position = 0,
	GL3_Attrib_Normal = 1,
	GL3_Attrib_Texcoord = 2,
	GL3_Attrib_Color = 3,
};


struct CGL3MeshBuffer : public CMeshBuffer
{
	CGL3MeshBuffer() = default;
	~CGL3MeshBuffer();
	//CGL3MeshBuffer(MeshBufferDesc &desc);

	void allocate(MeshBufferInit &init);
	void setupVAO();
	void updateVertex(void const *vertexData, std::size_t offset, std::size_t size);
	void updateIndex(void const *indexData, std::size_t offset, std::size_t size);
	void destroy();
	void draw();

	bool isValid() const {
		return mVBO != -1;
	}

	GLuint mVBO = -1;
	std::size_t mVBOSize = 0;
	GLuint mIBO = -1;
	std::size_t mIBOSize = 0;
	GLuint mVAO = -1;
};

#endif