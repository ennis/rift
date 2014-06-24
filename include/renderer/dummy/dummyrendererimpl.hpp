#ifndef DUMMYRENDERERIMPL_HPP
#define DUMMYRENDERERIMPL_HPP

#include <renderer.hpp>

class CDummyRendererImpl : public CRendererImplBase
{
public:
	void initialize();
	CTexture *createTexture(TextureDesc &desc);
	CMeshBuffer *createMeshBuffer(MeshBufferInit &init);
	void submit(CMeshBuffer *meshBuffer, Transform &transform);
	void render(RenderData &renderData);

	void setClearColor(glm::vec4 const &color);
	void setClearDepth(float color);
};

#endif