#include <mesh.hpp>
#include <renderer.hpp>

void CMesh::destroy()
{
	mMeshBuffer->destroy();
	delete this;
}
