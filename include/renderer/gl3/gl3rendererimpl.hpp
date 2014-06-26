#ifndef GL3RENDERERIMPL_HPP
#define GL3RENDERERIMPL_HPP

#include <renderer.hpp>
#include <gl3shader.hpp>
#include <vector>

struct CGL3MeshBuffer;

typedef Handle<CGL3MeshBuffer> CGL3MeshBufferRef;

struct Submission
{
	Submission(CGL3MeshBufferRef meshBuffer, CMaterialRef material, Transform &transform) : mMeshBuffer(meshBuffer), mMaterial(material), mModelTransform(transform)
	{}

	CGL3MeshBufferRef mMeshBuffer;
	CMaterialRef mMaterial;
	Transform mModelTransform;
};

class CGL3RendererImpl : public CRendererImplBase
{
public:
	void initialize();
	CTextureRef createTexture(TextureDesc &desc, std::string name);
	CMeshBufferRef createMeshBuffer(MeshBufferInit &init, std::string name);
	void submit(CMeshBufferRef meshBuffer, Transform &transform, CMaterialRef material);
	void render(RenderData &renderData);

	void setClearColor(glm::vec4 const &color);
	void setClearDepth(float color);

private:
	std::vector<Submission> mSubmissions;
	GLProgram mVertexColorUnlitProgram;

	GLuint mRenderDataUBO;

	glm::vec4 mClearColor = glm::vec4(226.f / 255.f, 236.f / 255.f, 124.f / 255.f, 1.f);
	float mClearDepth = 100.f;
};

#endif