#ifndef DUMMYRENDERERIMPL_HPP
#define DUMMYRENDERERIMPL_HPP

#include <renderer.hpp>

class CDummyRendererImpl : public CRendererImplBase
{
public:
	void initialize();
	CTexture2D *createTexture2D(Texture2DDesc &desc, const void *initialData) override;
	CTextureCubeMap *createTextureCubeMap(TextureCubeMapDesc &desc, const void *initialData[6]) override;
	CMeshBuffer *createMeshBuffer(MeshBufferInit &init);
	void submit(CMeshBuffer *meshBuffer, Transform &transform);
	void render(RenderData &renderData);

	void setClearColor(glm::vec4 const &color);
	void setClearDepth(float color);
};

#endif