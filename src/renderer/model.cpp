#include <model.hpp>
#include <log.hpp>
#include <renderer.hpp>

void CModel::addMeshPart(CMeshBuffer *meshBuffer, CMaterial *material)
{
	MeshPart part;
	part.mMeshBuffer = meshBuffer;
	part.mMaterial = material;
	mMeshParts.push_back(part);
}

void CModel::render(Transform &transform) 
{
	auto &renderer = CRenderer::getInstance();
	for (auto &part : mMeshParts) {
		auto &impl = renderer.getImpl();
		impl.submit(part.mMeshBuffer, transform, part.mMaterial);
	}
}

void CModel::deleteResource()
{
	delete this;
}