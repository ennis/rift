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
	mRenderData.lightDir = glm::normalize(glm::vec4(-1.f, -1.f, 0.f, 0.f));
}

void CRenderer::setCamera(Camera &camera, Transform &transform)
{
	mRenderData.eyePos = glm::vec4(transform.position, 1.f);
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

CMaterial *CRenderer::createMaterial(MaterialDesc &desc)
{
	assert(mImpl);
	return mImpl->createMaterial(desc);
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