#ifndef GL3RENDERERIMPL_HPP
#define GL3RENDERERIMPL_HPP

#include <renderer.hpp>
#include <gl3shader.hpp>
#include <vector>

struct CGL3MeshBuffer;

struct Submission
{
	Submission(CGL3MeshBuffer *meshBuffer, Transform &transform) : mMeshBuffer(meshBuffer), mModelTransform(transform)
	{}

	CGL3MeshBuffer *mMeshBuffer;
	Transform mModelTransform;
};

class CGL3RendererImpl : public CRendererImplBase
{
public:
	void initialize();
	CTexture *createTexture(TextureDesc &desc);
	CMeshBuffer *createMeshBuffer(MeshBufferInit &init);
	void submit(CMeshBuffer *meshBuffer, Transform &transform);
	void render(RenderData &renderData);

	void setClearColor(glm::vec4 const &color);
	void setClearDepth(float color);

private:
	std::vector<Submission> mSubmissions;
	GLProgram mVertexColorUnlitProgram;

	glm::vec4 mClearColor = glm::vec4(226.f / 255.f, 236.f / 255.f, 124.f / 255.f, 1.f);
	float mClearDepth = 100.f;
};

#endif