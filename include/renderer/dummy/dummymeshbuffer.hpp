#ifndef DUMMYMESHBUFFER_HPP
#define DUMMYMESHBUFFER_HPP

#include <meshbuffer.hpp>

struct CDummyMeshBuffer : public CMeshBuffer 
{
	CDummyMeshBuffer(MeshBufferDesc &desc);
	
	void updateVertex(void const *vertexData, std::size_t offset, std::size_t size);
	void updateIndex(void const *vertexData, std::size_t offset, std::size_t size);
	void destroy();
};

#endif