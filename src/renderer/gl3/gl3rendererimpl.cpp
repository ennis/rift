#include <gl3rendererimpl.hpp>
#include <gl3meshbuffer.hpp>
#include <gl3texture.hpp>
#include <gl3error.hpp>
#include <log.hpp>
#include <gl3shader.hpp>


// hopefully std140
struct RenderDataStd140
{
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	glm::mat4 viewProjMatrix;
	glm::vec3 lightDir;
	glm::ivec2 viewportSize;
};

static const std::size_t GL3_RenderData_Size = sizeof(RenderDataStd140);
static const int GL3_RenderData_Binding = 0;	// bind to location 0

struct MaterialParametersStd140
{

};

static GLuint createUniformBuffer(std::size_t size)
{
	GLuint obj;
	GLCHECK(glGenBuffers(1, &obj));
	GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, obj));
	GLCHECK(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW));
	GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	return obj;
}

void CGL3RendererImpl::initialize()
{
	// chargement des shaders
	mVertexColorUnlitProgram.loadFromFile(
		"resources/shaders/vertex_color_unlit.vert.glsl", 
		"resources/shaders/vertex_color_unlit.frag.glsl");

	// create uniform buffer
	mRenderDataUBO = createUniformBuffer(GL3_RenderData_Size);

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

	//
	RenderDataStd140 uboData;
	uboData.projMatrix = renderData.mProjMatrix;
	uboData.viewMatrix = renderData.mViewMatrix;
	uboData.viewProjMatrix = renderData.mProjMatrix * renderData.mViewMatrix;
	uboData.viewportSize = renderData.mViewportSize;

	// update uniform buffer
	GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, mRenderDataUBO));
	GLCHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, GL3_RenderData_Size, &uboData));
	GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));

	mVertexColorUnlitProgram.use();
	mVertexColorUnlitProgram.uniformMatrix4fv("viewProjMatrix", renderData.mProjMatrix * renderData.mViewMatrix);
	// bind render data
	// TODO do this once for each shader...
	GLuint blockIndex = mVertexColorUnlitProgram.getUniformBlockIndex("RenderData");
	GLCHECK(glUniformBlockBinding(mVertexColorUnlitProgram.program, blockIndex, GL3_RenderData_Binding));
	GLCHECK(glBindBufferRange(GL_UNIFORM_BUFFER, GL3_RenderData_Binding, mRenderDataUBO, 0, GL3_RenderData_Size));

	// ignore materials for now
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

void CGL3RendererImpl::submit(CMeshBuffer *meshBuffer, Transform &transform, CMaterial *material)
{
	mSubmissions.emplace_back(static_cast<CGL3MeshBuffer*>(meshBuffer), material, transform);
}