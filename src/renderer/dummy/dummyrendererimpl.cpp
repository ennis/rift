#include <dummyrendererimpl.hpp>
#include <dummytexture.hpp>
#include <dummymeshbuffer.hpp>
#include <log.hpp>

void CDummyRendererImpl::initialize()
{

}

CTexture *CDummyRendererImpl::createTexture(TextureDesc &desc)
{
	return new CDummyTexture(desc);
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