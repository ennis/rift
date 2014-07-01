#ifndef MESH_HPP
#define MESH_HPP

#include <meshbuffer.hpp>
#include <renderer.hpp>

struct CMesh : public CRenderResource
{
	CMesh(CMeshBuffer *meshBuffer) : mMeshBuffer(meshBuffer)
	{}

	virtual ~CMesh() {}

	void destroy();

	// owned
	CMeshBuffer *mMeshBuffer;
};


#endif