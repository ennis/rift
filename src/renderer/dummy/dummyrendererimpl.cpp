#include <dummyrendererimpl.hpp>
#include <dummytexture.hpp>
#include <dummymeshbuffer.hpp>
#include <log.hpp>

void CDummyRendererImpl::initialize()
{

}

CTexture2D *CDummyRendererImpl::createTexture2D(Texture2DDesc &desc, const void *initialData)
{
	return nullptr;
}

CTextureCubeMap *CDummyRendererImpl::createTextureCubeMap(TextureCubeMapDesc &desc, const void *initialData[6])
{
	return nullptr;
}

CMeshBuffer *CDummyRendererImpl::createMeshBuffer(MeshBufferInit &init)
{
	return new CDummyMeshBuffer(init.desc); 
}

void CDummyRendererImpl::submit(CMeshBuffer *meshBuffer, Transform &transform)
{
	LOG << "dummy submit";
}

void CDummyRendererImpl::render(RenderData &renderData)
{
	LOG << "dummy render";
}

void CDummyRendererImpl::setClearColor(glm::vec4 const &color)
{
}

void CDummyRendererImpl::setClearDepth(float color)
{

}