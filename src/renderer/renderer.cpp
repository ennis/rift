#include <renderer.hpp>
#include <mesh.hpp>

#include <mesh.hpp>
#include <texture.hpp>
#include <camera.hpp>
#include <material.hpp>
#include <model.hpp>
#include <game.hpp>

void CRenderer::initialize(std::unique_ptr<CRendererImplBase> impl)
{
	mImpl = std::move(impl);
	sInstance = this;
	mImpl->initialize();
	// TODO c'est laid
	mRenderData.mViewportSize = Game::getSize();
}

void CRenderer::setCamera(Camera &camera)
{
	mRenderData.mProjMatrix = camera.projMatrix;
	mRenderData.mViewMatrix = camera.viewMatrix;
}

CMesh *CRenderer::createMesh(MeshBufferInit &init)
{
	// create mesh buffer
	assert(mImpl);
	CMeshBuffer *meshBuffer = mImpl->createMeshBuffer(init);
	return new CMesh(meshBuffer);
}

CModel *CRenderer::createModel()
{
	return new CModel();
}

CTexture2D *CRenderer::createTexture2D(Texture2DDesc &desc, const void *initialData)
{
	assert(mImpl);
	return mImpl->createTexture2D(desc, initialData);
}

CTextureCubeMap *CRenderer::createTextureCubeMap(TextureCubeMapDesc &desc, const void *initialData[6])
{
	assert(mImpl);
	return mImpl->createTextureCubeMap(desc, initialData);
}

void CRenderer::render(CMesh *mesh, Transform &transform)
{
	mImpl->submit(mesh->mMeshBuffer, transform, nullptr);
}

void CRenderer::render(CModel *model, Transform &transform)
{
	for (auto &part : model->mMeshParts) {
		mImpl->submit(part.mMeshBuffer.get(), transform, part.mMaterial.get());
	}
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