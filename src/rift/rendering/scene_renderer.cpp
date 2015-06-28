#include <rendering/scene_renderer.hpp>
#include <rendering/opengl4.hpp>
#include <rendering/nanovg/nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <rendering/nanovg/nanovg_gl.h>
#include <image.hpp>
#include <log.hpp>

using namespace glm;

namespace
{
	struct PerObject
	{
		glm::mat4 modelToWorld;
	};

	Buffer *createSceneViewUBO(GraphicsContext &gc, const SceneView &sv)
	{
		auto tbuf = gc.createTransientBuffer<SceneView>();
		*(tbuf.map()) = sv;
		return tbuf.buf;
	}

	struct LightParams
	{
		glm::vec4 intensity;
		union
		{
			float center[4]; 
			float direction[4];
		} u;
	};
}


SceneRenderer::SceneRenderer(glm::ivec2 viewportSize_, GraphicsContext &gc, AssetDatabase &assetDb) :
	viewportSize(viewportSize_),
	graphicsContext(gc)
{
	defaultMaterial = std::make_unique<Material>();
	defaultMaterial->shader = loadShaderAsset(assetDb, gc, "resources/shaders/default.glsl");
	defaultMaterial->diffuseMap = loadTexture2DAsset(assetDb, gc, "resources/img/default.tga");
	defaultMaterial->normalMap = nullptr;
	defaultMaterial->userParams = nullptr;
	defaultFont = Font::loadFromFile("resources/img/fonts/debug.fnt");
	nvgContext = nvgCreateGL3(NVG_ANTIALIAS);

	// text VAO
	textVao.create(1, { Attribute{ ElementFormat::Float4 } });
	// text shader
	{
		auto src = loadShaderSource("resources/shaders/text.glsl");
		auto vs = compileShader(src.c_str(), "", gl::VERTEX_SHADER, {});
		auto ps = compileShader(src.c_str(), "", gl::FRAGMENT_SHADER, {});
		textProgram = compileProgram(vs, 0, ps);
		gl::DeleteShader(vs);
		gl::DeleteShader(ps);
	}

	// Meshes
	meshVao.create(1, { 
		{ ElementFormat::Float3, 0 },
		{ ElementFormat::Snorm10x3_1x2, 0 },
		{ ElementFormat::Snorm10x3_1x2, 0 },
		{ ElementFormat::Unorm16x2, 0 },
	});

	setDebugCallback();
}

void SceneRenderer::renderScene(Scene &scene, const Camera &camera, float dt)
{
	if (scene.lightNodes.empty())
		WARNING << "no lights!";
	scene.lastFrameTimes[scene.lastFrameIndex] = dt;

	// update scene data buffer
	SceneView sceneView;
	sceneView.wEye = vec4(camera.wEye, 1.0f);
	sceneView.lightDir = vec4(0.0, 1.0f, 1.0f, 0.0f);
	sceneView.projMatrix = camera.projMat;
	sceneView.viewMatrix = camera.viewMat;
	sceneView.viewProjMatrix = camera.projMat * camera.viewMat;
	sceneView.viewportSize = viewportSize;

	ForwardPass pass;
	pass.sceneView = &sceneView;
	pass.sceneViewUBO = createSceneViewUBO(graphicsContext, sceneView);

	// flatten entity hierarchy
	for (auto &t : scene.transforms) {
		EntityID ent = t.second.parent;
		glm::mat4 flatTransform = t.second.transform.toMatrix();
		while (ent != -1) {
			flatTransform = scene.transforms[ent].transform.toMatrix() * flatTransform;
			ent = scene.transforms[ent].parent;
		}
		scene.flattenedTransforms[t.first] = flatTransform;
	}

	// begin render commands
	gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
	gl::Viewport(0, 0, viewportSize.x, viewportSize.y);
	gl::ClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	gl::ClearDepth(1.0f);
	gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
	// stencil test
	gl::Disable(gl::STENCIL_TEST);
	gl::Enable(gl::CULL_FACE);
	gl::CullFace(gl::BACK);
	gl::PolygonMode(gl::FRONT_AND_BACK, gl::FILL);
	gl::Enable(gl::DEPTH_TEST);
	gl::DepthFunc(gl::LEQUAL);
	gl::DepthMask(gl::TRUE_);
	// alpha blending ON
	gl::Enable(gl::BLEND);
	gl::BlendEquationSeparatei(0,gl::FUNC_ADD, gl::FUNC_ADD);
	gl::BlendFuncSeparatei(0, gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA, gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);

	// Fwd pass for each light
	for (auto &l : scene.lightNodes)
	{
		auto &lightNode = l.second;
		auto &lightTransform = scene.transforms[l.first].transform;
		auto lightbuf = graphicsContext.createTransientBuffer<LightParams>();
		auto plight = lightbuf.map();
		plight->intensity = vec4(lightNode.light.intensity, 1.0f);

		pass.light = &lightNode.light;
		pass.lightParamsUBO = lightbuf.buf;

		// Buffer for light parameters
		if (lightNode.light.mode == LightMode::Directional)
		{
			plight->u.direction[0] = 0.0f;
			plight->u.direction[1] = 1.0f;
			plight->u.direction[2] = 0.0f;
			plight->u.direction[3] = 1.0f;
		}
		else if (lightNode.light.mode == LightMode::Point)
		{
			plight->u.direction[0] = lightTransform.position.x;
			plight->u.direction[1] = lightTransform.position.y;
			plight->u.direction[2] = lightTransform.position.z;
			plight->u.direction[3] = 1.0f;
		}
		else if (lightNode.light.mode == LightMode::Spot)
		{
			// TODO
		}

		// main pass
		for (auto &meshEntity : scene.meshNodes) {
			auto &meshNode = meshEntity.second;
			auto &transform = scene.flattenedTransforms[meshEntity.first];
			drawMeshForwardPass(
				pass,
				*meshNode.mesh,
				meshNode.material ? *meshNode.material : *defaultMaterial,
				transform);
		}
	}

	// EXTRAS/DEBUG
	//nvgBeginFrame(nvgContext, viewportSize.x, viewportSize.y, 1.0f);
	drawScreenMessages();
	//drawFrameTimeGraph(scene);
	//nvgEndFrame(nvgContext);
	scene.lastFrameIndex = (scene.lastFrameIndex + 1) % scene.lastFrameTimes.size();
}


void SceneRenderer::prepareMaterialForwardPass(
	Material &mat,
	ForwardPass &pass, 
	const glm::mat4 &modelToWorld)
{
	if (&mat == pass.lastMaterial)
		return;
	pass.lastMaterial = &mat;
	// XXX replace with direct OpenGL calls
	GLuint program;
	if (pass.light->mode == LightMode::Directional)
		program = mat.shader->programForwardDirectionalLight;
	else if (pass.light->mode == LightMode::Point)
		program = mat.shader->programForwardPointLight;
	else if (pass.light->mode == LightMode::Spot)
		program = mat.shader->programForwardSpotLight;
	else
		assert(!"Unsupported light mode");

	if (pass.lastProgram != program) {
		gl::UseProgram(program);
		pass.lastProgram = program;
	}

	GLuint textures[2];
	GLuint samplers[2];
	textures[0] = mat.diffuseMap->getGL();
	samplers[0] = graphicsContext.getSamplerLinearClamp();

	if (mat.normalMap) {
		textures[1] = mat.normalMap->getGL();
		samplers[1] = samplers[0];
		gl::BindTextures(0, 2, textures);
		gl::BindSamplers(0, 2, samplers);
	} else {
		gl::BindTextures(0, 1, textures);
		gl::BindSamplers(0, 1, samplers);
	}
	
	// Per-object uniforms
	auto perObjBuf = graphicsContext.createTransientBuffer<PerObject>();
	perObjBuf.map()->modelToWorld = modelToWorld;
	if (!mat.userParams)
		bindBuffersRangeHelper(0, { pass.sceneViewUBO, pass.lightParamsUBO, perObjBuf.buf });
	else
		bindBuffersRangeHelper(0, { pass.sceneViewUBO, pass.lightParamsUBO, perObjBuf.buf, mat.userParams });
}


void SceneRenderer::drawMeshForwardPass(
	ForwardPass &pass,
	Mesh &mesh, 
	Material &material,
	const glm::mat4 &modelToWorld)
{
	prepareMaterialForwardPass(material, pass, modelToWorld);
	bindVertexBuffers({ mesh.vbo.get() }, meshVao);
	for (auto &sm : mesh.submeshes)
		drawIndexed(gl::TRIANGLES, *mesh.ibo, sm.startVertex, sm.startIndex, sm.numIndices, 0, 1);
}

void SceneRenderer::drawScreenMessages()
{
	auto lines = Logging::clearScreenMessages();
	unsigned ypos = 5;
	unsigned xpos = 5;
	unsigned yinc = defaultFont->getMetrics().height;
	for (auto &&line : lines) {
		drawTextShadow(
			vec2(xpos, ypos),
			line.c_str());
		ypos += yinc;
	}
}

void SceneRenderer::drawFrameTimeGraph(Scene &scene)
{
	float posX = 0.0f;
	float posY = 0.0f;
	float curX = posX;
	const float barWidth = 3.0f;
	const float barHeightScale = 1000.f;
	auto numPoints = scene.lastFrameTimes.size();
	for (unsigned ii = 0; ii < numPoints; ++ii)
	{
		nvgBeginPath(nvgContext);
		const float barHeight = barHeightScale * scene.lastFrameTimes[ii];
		nvgRect(nvgContext, curX, posY - barHeight, barWidth, barHeight);
		curX += barWidth;
		if (ii == scene.lastFrameIndex)
			nvgFillColor(nvgContext, nvgRGBA(255, 70, 70, 255));
		else
			nvgFillColor(nvgContext, nvgRGBA(70, 70, 255, 255));
		nvgFill(nvgContext);
	}
}