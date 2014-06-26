#ifndef MESH_HPP
#define MESH_HPP

#include <meshbuffer.hpp>
#include <renderer.hpp>
#include <renderresource.hpp>

struct CMesh : public CRenderResource
{
	CMesh(CMeshBufferRef meshBuffer) : mMeshBuffer(meshBuffer)
	{}

	virtual ~CMesh() {}

	void destroy();

	CMeshBufferRef mMeshBuffer;
};


#endif