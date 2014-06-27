#include <dummymeshbuffer.hpp>
#include <log.hpp>

CDummyMeshBuffer::CDummyMeshBuffer(MeshBufferDesc &desc)
{
	LOG << "dummy mesh buffer";
}

void CDummyMeshBuffer::destroy()
{
	LOG << "dummy delete";
	delete this;
}


void CDummyMeshBuffer::updateVertex(void const *vertexData, std::size_t offset, std::size_t size)
{
	LOG << "vertexData=" << vertexData << " offset=" << offset << " size=" << size;
}

void CDummyMeshBuffer::updateIndex(void const *indexData, std::size_t offset, std::size_t size)
{
	LOG << "indexData=" << indexData << " offset=" << offset << " size=" << size;
}
