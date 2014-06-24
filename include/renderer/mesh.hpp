#ifndef MESH_HPP
#define MESH_HPP

#include <renderable.hpp>
#include <meshbuffer.hpp>

struct CMesh : public CRenderable
{
	CMesh(CMeshBuffer *meshBuffer) : mMeshBuffer(meshBuffer)
	{}

	virtual ~CMesh() {}

	void render(Transform &transform);

	void deleteResource();

	CMeshBuffer *mMeshBuffer;
};


#endif