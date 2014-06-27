#ifndef GL3RENDERERIMPL_HPP
#define GL3RENDERERIMPL_HPP

#include <renderer.hpp>
#include <gl3shader.hpp>
#include <vector>

struct CGL3MeshBuffer;
struct CGL3Texture2D;
struct CGL3TextureCubeMap;

typedef Handle<CGL3MeshBuffer> CGL3MeshBufferRef;

struct Submission
{
	Submission(CGL3MeshBuffer *meshBuffer, CMaterial *material, Transform &transform) : mMeshBuffer(meshBuffer), mMaterial(material), mModelTransform(transform)
	{}

	CGL3MeshBuffer *mMeshBuffer;
	CMaterial *mMaterial;
	Transform mModelTransform;
};

class CGL3RendererImpl : public CRendererImplBase
{
public:
	void initialize();

	CTexture2D *createTexture2D(Texture2DDesc &desc, const void *initialData) override;
	CTextureCubeMap *createTextureCubeMap(TextureCubeMapDesc &desc, const void *initialData[6]) override;

	CMeshBuffer *createMeshBuffer(MeshBufferInit &init) override;
	void submit(CMeshBuffer *meshBuffer, Transform &transform, CMaterial *material) override;
	void render(RenderData &renderData);

	void setClearColor(glm::vec4 const &color);
	void setClearDepth(float color);
	void debugBlit(CGL3Texture2D *texture, glm::ivec2 screenPos, float scale);
	void bindRenderData(GLProgram &prog);

private:
	std::vector<Submission> mSubmissions;
	GLProgram mVertexColorUnlitProgram;
	GLProgram mDefaultProgram;

	GLuint mRenderDataUBO;
	CGL3MeshBuffer *mScreenQuad;
	GLProgram mBlitProgram;
	CTexture2D *mDefaultTexture;
	CTextureCubeMap *mDefaultCubeMap;
	CTextureCubeMapRef mDefaultEnvmap;

	// default sampler object (nearest, no mip maps)
	GLuint mBlitSampler;

	glm::vec4 mClearColor = glm::vec4(226.f / 255.f, 236.f / 255.f, 124.f / 255.f, 1.f);
	float mClearDepth = 100.f;
};

#endif