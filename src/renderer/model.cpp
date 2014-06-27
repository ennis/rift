#include <model.hpp>
#include <log.hpp>
#include <renderer.hpp>

void CModel::addMeshPart(CMeshBufferRef meshBuffer, CMaterialRef material)
{
	MeshPart part;
	part.mMeshBuffer = meshBuffer;
	part.mMaterial = material;
	mMeshParts.push_back(part);
}

void CModel::destroy()
{
	delete this;
}