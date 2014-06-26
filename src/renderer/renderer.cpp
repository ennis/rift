#include <renderer.hpp>
#include <mesh.hpp>

#include <mesh.hpp>
#include <texture.hpp>
#include <camera.hpp>
#include <material.hpp>
#include <model.hpp>

void CRenderer::initialize(std::unique_ptr<CRendererImplBase> impl)
{
	mImpl = std::move(impl);
	sInstance = this;
	mImpl->initialize();
}

void CRenderer::setCamera(Camera &camera)
{
	mRenderData.mProjMatrix = camera.projMatrix;
	mRenderData.mViewMatrix = camera.viewMatrix;
}

CMeshRef CRenderer::createMesh(MeshBufferInit &init, std::string name)
{
	// create mesh buffer
	assert(mImpl);
	CMeshBufferRef meshBuffer = mImpl->createMeshBuffer(init, name + "!buffer");
	return addRenderResource(new CMesh(meshBuffer), name);
}

CModelRef CRenderer::createModel(std::string name)
{
	return addRenderResource(new CModel(), name);
}

CTextureRef CRenderer::createTexture(TextureDesc &desc, std::string name)
{
	assert(mImpl);
	return mImpl->createTexture(desc);
}

void CRenderer::render(CMeshRef mesh, Transform &transform)
{
	auto meshPtr = mesh.lock();
	mImpl->submit(meshPtr->mMeshBuffer, transform, nullptr);
	mesh.unlock();
}

void CRenderer::render(CModelRef model, Transform &transform)
{
	auto modelPtr = model.lock();
	for (auto &part : modelPtr->mMeshParts) {
		mImpl->submit(part.mMeshBuffer, transform, part.mMaterial);
	}
	model.unlock();
}

void CRenderer::render()
{
	assert(mImpl);
	mImpl->render(mRenderData);
}

CRendererImplBase &CRenderer::getImpl()
{
	return *mImpl.get();
}

CRenderer *CRenderer::sInstance;