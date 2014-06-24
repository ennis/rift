#include <renderer.hpp>
#include <mesh.hpp>

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


CMesh *CRenderer::createMesh(MeshBufferInit &init)
{
	// create mesh buffer
	assert(mImpl);
	CMeshBuffer *meshBuffer = mImpl->createMeshBuffer(init);
	return new CMesh(meshBuffer);
}

CTexture *CRenderer::createTexture(TextureDesc &desc)
{
	assert(mImpl);
	return mImpl->createTexture(desc);
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