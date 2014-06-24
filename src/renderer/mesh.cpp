#include <mesh.hpp>
#include <renderer.hpp>

void CMesh::deleteResource()
{
	mMeshBuffer->deleteResource();
	delete this;
}

void CMesh::render(Transform &transform)
{
	auto &impl = CRenderer::getInstance().getImpl();
	impl.submit(mMeshBuffer, transform);
}