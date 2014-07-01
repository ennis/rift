#include <gl3rendererimpl.hpp>
#include <gl3meshbuffer.hpp>
#include <gl3texture.hpp>
#include <gl3error.hpp>
#include <log.hpp>
#include <gl3shader.hpp>
#include <gl3material.hpp>
#include <textureloader.hpp>


// hopefully std140
struct RenderDataStd140
{
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	glm::mat4 viewProjMatrix;
	glm::vec4 eyePos;
	glm::vec4 lightDir;
	glm::vec2 viewportSize;
};

static const std::size_t GL3_RenderData_Size = sizeof(RenderDataStd140);
static const int GL3_RenderData_Binding = 0;	// bind to location 0

class CGL3ResourceLoader : public ResourceLoader
{
	void *load(std::string key) override {
		throw std::logic_error("load");
	}

	void destroy(std::string const &key, void *resource) override {
		static_cast<CRenderResource*>(resource)->destroy();
	}
};

enum GL3TextureUnits
{
	GL3_Texture_Envmap = 0,
	GL3_Texture_DiffuseMap,
	GL3_Texture_NormalMap,
	GL3_Texture_SpecularMap,
	GL3_Texture_RoughnessMap
};

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

void CGL3RendererImpl::bindRenderData(GLProgram &prog)
{
	GLuint blockIndex = prog.getUniformBlockIndex("RenderData");
	GLCHECK(glUniformBlockBinding(prog.program, blockIndex, GL3_RenderData_Binding));
	GLCHECK(glBindBufferRange(GL_UNIFORM_BUFFER, GL3_RenderData_Binding, mRenderDataUBO, 0, GL3_RenderData_Size));
}

void CGL3RendererImpl::initialize()
{
	// chargement des shaders
	mVertexColorUnlitProgram.loadFromFile(
		"resources/shaders/vertex_color_unlit.vert.glsl", 
		"resources/shaders/vertex_color_unlit.frag.glsl");

	mDefaultProgram.loadFromFile(
		"resources/shaders/pbs.vert.glsl",
		"resources/shaders/pbs.frag.glsl"
		);

	mBlitProgram.loadFromFile(
		"resources/shaders/blit.vert.glsl",
		"resources/shaders/blit.frag.glsl");

	// create uniform buffer
	mRenderDataUBO = createUniformBuffer(GL3_RenderData_Size);

	static const float positions[] = {
		0.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	// create screen quad
	MeshBufferInit mbi;
	mbi.positions = positions;
	mbi.desc.numVertices = 6;
	mbi.desc.layoutType = Layout_Full;
	mbi.desc.primitiveType = PrimitiveType::Triangles;
	mScreenQuad = new CGL3MeshBuffer();
	mScreenQuad->allocate(mbi);

	// default sampler
	GLCHECK(glGenSamplers(1, &mBlitSampler));
	GLCHECK(glSamplerParameteri(mBlitSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
	GLCHECK(glSamplerParameteri(mBlitSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCHECK(glSamplerParameteri(mBlitSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GLCHECK(glSamplerParameteri(mBlitSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCHECK(glSamplerParameteri(mBlitSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	// default textures
	static const uint8_t data[] = { 128, 128, 128, 255 };
	static const void *faceData[] = { data, data, data, data, data, data };

	Texture2DDesc defaultTexture2DDesc;
	defaultTexture2DDesc.format = PixelFormat::R8G8B8A8;
	defaultTexture2DDesc.size = glm::ivec2(1, 1);
	defaultTexture2DDesc.numMipMapLevels = 1; 

	mEmptyTexture = make_handle<CTexture2D>(
		createTexture2D(defaultTexture2DDesc, data), 
		std::unique_ptr<ResourceLoader>(new CGL3ResourceLoader()), 
		"<default texture>");

	mDefaultTexture = loadTexture2DFromFile("resources/img/default.tga");

	TextureCubeMapDesc defaultTextureCubeMapDesc;
	defaultTextureCubeMapDesc.format = PixelFormat::R8G8B8A8;
	defaultTextureCubeMapDesc.size = glm::ivec2(1, 1);
	defaultTextureCubeMapDesc.numMipMapLevels = 1;
	mDefaultCubeMap = createTextureCubeMap(defaultTextureCubeMapDesc, faceData);

	// envmap
	mDefaultEnvmap = loadTextureCubeMapFromFile(
		"resources/img/env/uffizi/1.jpg",
		"resources/img/env/uffizi/2.jpg",
		"resources/img/env/uffizi/3.jpg",
		"resources/img/env/uffizi/4.jpg",
		"resources/img/env/uffizi/5.jpg",
		"resources/img/env/uffizi/6.jpg");

	// default material
	MaterialDesc defaultMaterialDesc;
	defaultMaterialDesc.diffuseMap = mDefaultTexture;
	defaultMaterialDesc.normalMap = mEmptyTexture;
	defaultMaterialDesc.specularMap = mEmptyTexture;
	defaultMaterialDesc.roughnessMap = mEmptyTexture;

	mDefaultMaterial = make_handle<CMaterial>(
		createMaterial(defaultMaterialDesc),
		std::unique_ptr<ResourceLoader>(new CGL3ResourceLoader()),
		"<default material>");
	
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
	//GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	GLCHECK(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

	//
	RenderDataStd140 uboData;
	uboData.projMatrix = renderData.mProjMatrix;
	uboData.viewMatrix = renderData.mViewMatrix;
	uboData.viewProjMatrix = renderData.mProjMatrix * renderData.mViewMatrix;
	uboData.viewportSize = renderData.mViewportSize;
	uboData.eyePos = renderData.eyePos;
	uboData.lightDir = renderData.lightDir;

	// update uniform buffer
	GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, mRenderDataUBO));
	GLCHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, GL3_RenderData_Size, &uboData));
	GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));

	// ignore materials for now
	for (auto &sub : mSubmissions) {
		prepareMaterial(static_cast<CGL3Material*>(sub.mMaterial));
		mDefaultProgram.uniformMatrix4fv("modelMatrix", sub.mModelTransform.toMatrix());
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

CTexture2D *CGL3RendererImpl::createTexture2D(Texture2DDesc &desc, const void *initialData)
{
	CGL3Texture2D *tex = new CGL3Texture2D(desc);
	if (initialData)
		tex->update(0, glm::ivec2(0, 0), desc.size, initialData);
	return tex;
}

CTextureCubeMap *CGL3RendererImpl::createTextureCubeMap(TextureCubeMapDesc &desc, const void *initialData[6])
{
	CGL3TextureCubeMap *tex = new CGL3TextureCubeMap(desc);
	for (int i = 0; i < 6; ++i) {
		tex->update(0, i, glm::ivec2(0, 0), desc.size, initialData[i]);
	}
	return tex;
}

CMeshBuffer *CGL3RendererImpl::createMeshBuffer(MeshBufferInit &init)
{
	auto meshBuffer = new CGL3MeshBuffer();
	meshBuffer->allocate(init);
	return meshBuffer;
}

CMaterial *CGL3RendererImpl::createMaterial(MaterialDesc &desc)
{
	return new CGL3Material(desc);
}

void CGL3RendererImpl::submit(CMeshBuffer *meshBuffer, Transform &transform, CMaterial *material)
{
	if (material == nullptr) {
		// TODO shouldn't be done here: material should not be NULL
		material = mDefaultMaterial.get();
	}
	mSubmissions.emplace_back(static_cast<CGL3MeshBuffer*>(meshBuffer), material, transform);
}

void CGL3RendererImpl::debugBlit(CGL3Texture2D *texture, glm::ivec2 screenPos, float scale)
{
	mBlitProgram.use();
	GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	bindRenderData(mBlitProgram);
	texture->setActive(0);
	mBlitProgram.uniform2f("screenPosition", screenPos);
	mBlitProgram.uniform2f("textureSize", scale * glm::vec2(texture->mDesc.size.x, texture->mDesc.size.y));
	GLCHECK(glBindSampler(0, mBlitSampler));

	mScreenQuad->draw();
}

void CGL3RendererImpl::prepareMaterial(CGL3Material *material)
{
	mDefaultProgram.use();
	bindRenderData(mDefaultProgram);
	// TODO do this once
	mDefaultProgram.uniform1i("gEnvmap", GL3_Texture_Envmap);
	mDefaultProgram.uniform1i("gDiffuse", GL3_Texture_DiffuseMap);
	mDefaultProgram.uniform1i("gNormal", GL3_Texture_NormalMap);
	mDefaultProgram.uniform1i("gSpecular", GL3_Texture_SpecularMap);
	mDefaultProgram.uniform1i("gRoughness", GL3_Texture_RoughnessMap);

	// bind textures
	auto envmap = static_cast<CGL3TextureCubeMap*>(mDefaultEnvmap.get());
	envmap->setActive(0);

	CGL3Texture2D *diffuseMap = static_cast<CGL3Texture2D*>(material->mDesc.diffuseMap.empty() ? mDefaultTexture.get() : material->mDesc.diffuseMap.get());
	CGL3Texture2D *normalMap = static_cast<CGL3Texture2D*>(material->mDesc.normalMap.empty() ? mEmptyTexture.get() : material->mDesc.normalMap.get());
	CGL3Texture2D *specularMap = static_cast<CGL3Texture2D*>(material->mDesc.specularMap.empty() ? mEmptyTexture.get() : material->mDesc.specularMap.get());
	CGL3Texture2D *roughnessMap = static_cast<CGL3Texture2D*>(material->mDesc.roughnessMap.empty() ? mEmptyTexture.get() : material->mDesc.roughnessMap.get());

	diffuseMap->setActive(GL3_Texture_DiffuseMap);
	normalMap->setActive(GL3_Texture_NormalMap);
	specularMap->setActive(GL3_Texture_SpecularMap);
	roughnessMap->setActive(GL3_Texture_RoughnessMap);

}