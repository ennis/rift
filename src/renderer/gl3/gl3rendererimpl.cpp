#include <gl3rendererimpl.hpp>
#include <gl3meshbuffer.hpp>
#include <gl3texture.hpp>
#include <gl3error.hpp>
#include <log.hpp>
#include <gl3shader.hpp>

void CGL3RendererImpl::initialize()
{

	// chargement des shaders
	mVertexColorUnlitProgram.loadFromFile(
		"resources/shaders/vertex_color_unlit.vert.glsl", 
		"resources/shaders/vertex_color_unlit.frag.glsl");

	LOG << "Initialized GL3 renderer";
}


void CGL3RendererImpl::render(RenderData &renderData)
{
	// état par défaut du pipeline
	GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	// 226.f / 255.f, 236.f / 255.f, 124.f / 255.f, 1.f
	GLCHECK(glClearColor(mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a));
	GLCHECK(glClearDepth(mClearDepth));
	GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	GLCHECK(glEnable(GL_DEPTH_TEST));
	GLCHECK(glDisable(GL_CULL_FACE));
	GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	GLCHECK(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
	mVertexColorUnlitProgram.use();
	mVertexColorUnlitProgram.uniformMatrix4fv("viewProjMatrix", renderData.mProjMatrix * renderData.mViewMatrix);

	for (auto &sub : mSubmissions) {
		mVertexColorUnlitProgram.uniformMatrix4fv("modelMatrix", sub.mModelTransform.toMatrix());
		sub.mMeshBuffer->draw();
	}

	mSubmissions.clear();
}

void CGL3RendererImpl::setClearColor(glm::vec4 const &color)
{
	mClearColor = color;
}

void CGL3RendererImpl::setClearDepth(float depth)
{
	mClearDepth = depth;
}

CTexture *CGL3RendererImpl::createTexture(TextureDesc &desc)
{
	return new CGL3Texture(desc);
}

CMeshBuffer *CGL3RendererImpl::createMeshBuffer(MeshBufferInit &init)
{
	auto meshBuffer = new CGL3MeshBuffer();
	meshBuffer->allocate(init);
	return meshBuffer;
}

void CGL3RendererImpl::submit(CMeshBuffer *meshBuffer, Transform &transform)
{
	mSubmissions.emplace_back(static_cast<CGL3MeshBuffer*>(meshBuffer), transform);
}