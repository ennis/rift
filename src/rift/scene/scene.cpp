#include <scene.hpp>
#include <rendering/opengl4/pass.hpp>
#include <colors.hpp>
#define NANOVG_GL3_IMPLEMENTATION
#include <rendering/opengl4/nanovg/nanovg.h>
#include <rendering/opengl4/nanovg/nanovg_gl.h>
#include <scene/frame_time_graph.hpp>

namespace
{
	constexpr unsigned NumLastFrameTimes = 200;
}

Scene::Scene(gl4::GraphicsContext &graphicsContext_, ResourceLoader &resourceLoader) :
graphicsContext(graphicsContext_),
meshRenderer(graphicsContext_),
textRenderer(graphicsContext_),
lastEntity(0),
lastFrameTimes(NumLastFrameTimes),
lastFrameIndex(0)
{
	defaultMaterial = std::make_unique<gl4::Material>();
	defaultMaterial->shader = resourceLoader.loadShader("resources/shaders/default.glsl", graphicsContext);
	defaultMaterial->diffuseMap = resourceLoader.loadTexture("resources/img/default.tga");
	defaultMaterial->normalMap = nullptr;
	defaultMaterial->userParams = nullptr;
	debugFont = Font::loadFromFile("resources/img/fonts/debug.fnt");
	std::fill(lastFrameTimes.begin(), lastFrameTimes.end(), 0.0f);
	nvgContext = nvgCreateGL3(NVG_ANTIALIAS);
}

Entity Scene::createEntity()
{
	auto id = lastEntity++;
	entities.push_back(id);
	transforms.insert(std::make_pair(id, TransformNode()));
	return id;
}

Entity Scene::createLightPrefab(const Transform &transform, gl4::LightMode lightMode, const glm::vec3 &intensity)
{
	auto id = createEntity();
	auto t = getTransformNode(id);
	t->transform = transform;
	auto light = CreateComponent(lightNodes, id);
	light->light.intensity = intensity;
	light->light.mode = lightMode;
	return id;
}

Entity Scene::createMeshPrefab(const Transform &transform, gl4::Mesh &mesh, gl4::Material &material)
{
	auto id = createEntity();
	auto t = getTransformNode(id);
	t->transform = transform;
	auto meshNode = CreateComponent(meshNodes, id);
	meshNode->entity = id;
	meshNode->material = &material;
	meshNode->mesh = &mesh;
	return id;
}

TransformNode *Scene::getTransformNode(Entity id)
{
	return &transforms[id];
}

MeshNode *Scene::createMeshNode(Entity id, gl4::Mesh &mesh, gl4::Material &material)
{
	auto m = CreateComponent(meshNodes, id);
	m->entity = id;
	m->material = &material;
	m->mesh = &mesh;
	return m;
}
	
LightNode *Scene::createPointLight(Entity id, const glm::vec3 &intensity)
{
	auto l = CreateComponent(lightNodes, id);
	l->light.intensity = intensity;
	l->light.mode = gl4::LightMode::Point;
	return l;
}

LightNode *Scene::createDirectionalLight(Entity id, const glm::vec3 &intensity)
{
	auto l = CreateComponent(lightNodes, id);
	l->light.intensity = intensity;
	l->light.mode = gl4::LightMode::Directional;
	return l;
}

util::array_ref<Entity> Scene::getEntities() const
{
	return util::make_array_ref(entities);
}

void Scene::deleteEntity(Entity id)
{
	LOG << "deleteEntity: TODO";
}

namespace
{
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

void Scene::render(Camera &camera, glm::ivec2 viewportSize, float dt)
{
	lastFrameTimes[lastFrameIndex] = dt;

	using namespace gl4;
	gl4::SceneView sceneView;
	gl4::ForwardPassContext pass;

	// update scene data buffer
	sceneView.wEye = glm::vec4(camera.wEye, 1.0f);
	sceneView.lightDir = glm::vec4(0.0, 1.0f, 1.0f, 0.0f);
	sceneView.projMatrix = camera.projMat;
	sceneView.viewMatrix = camera.viewMat;
	sceneView.viewProjMatrix = camera.projMat * camera.viewMat;
	sceneView.viewportSize = viewportSize;

	gl4::CommandBuffer cmdBuf;
	cmdBuf.setScreenRenderTarget();
	float cc[4] = { 0.2f, 0.8f, 0.1f, 1.0f };
	cmdBuf.clearColor(cc);
	cmdBuf.clearDepth(1.0f);

	// TODO for each light:
	pass.sceneView = &sceneView;
	pass.sceneViewUBO = createSceneViewUBO(graphicsContext, sceneView);
	pass.cmdBuf = &cmdBuf;

	if (lightNodes.empty())
	{
		WARNING << "no lights!";
	}

	// flatten entity hierarchy
	for (auto &t : transforms)
	{
		Entity ent = t.second.parent;
		glm::mat4 flatTransform = t.second.transform.toMatrix();
		while (ent != -1)
		{
			flatTransform = transforms[ent].transform.toMatrix() * flatTransform;
			ent = transforms[ent].parent;
		}
		flattenedTransforms[t.first] = flatTransform;
	}

	for (auto &l : lightNodes)
	{
		auto &lightNode = l.second;
		auto &lightTransform = transforms[l.first].transform;
		auto tbuf = graphicsContext.allocTransientBuffer<LightParams>();
		auto plight = tbuf.map();
		plight->intensity = glm::vec4(lightNode.light.intensity, 1.0f);

		pass.light = &lightNode.light;
		pass.lightParamsUBO = tbuf.buf;

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
		for (auto &meshEntity : meshNodes)
		{
			auto &meshNode = meshEntity.second;
			auto &transform = flattenedTransforms[meshEntity.first];
			meshRenderer.renderForwardPass(
				pass,
				*meshNode.mesh,
				*meshNode.material,
				transform);
		}
	}

	// TODO move this in its own file (scene/debug_view.cpp)
	auto lines = Logging::clearScreenMessages();
	unsigned ypos = 5;
	unsigned xpos = 5;
	unsigned yinc = debugFont->getMetrics().height;
	for (auto &&line : lines)
	{
		// drop shadow FTW
		textRenderer.render(
			cmdBuf,
			line,
			*debugFont,
			glm::vec2(xpos+2, ypos+2),
			glm::vec2(viewportSize.x, viewportSize.y),
			Color::Black,
			glm::vec4(0.0, 0.0, 0.0, 0.0));
		textRenderer.render(
			cmdBuf,
			line,
			*debugFont,
			glm::vec2(xpos, ypos),
			glm::vec2(viewportSize.x, viewportSize.y),
			Color::White,
			glm::vec4(0.0, 0.0, 0.0, 0.0));
		ypos += yinc;
	}

	graphicsContext.execute(cmdBuf);

	nvgBeginFrame(nvgContext, viewportSize.x, viewportSize.y, 1.0f);
	RenderFrameTimeGraph(nvgContext, 0.f, viewportSize.y, util::make_array_ref(lastFrameTimes), lastFrameIndex);
	nvgEndFrame(nvgContext);

	lastFrameIndex = (lastFrameIndex + 1) % NumLastFrameTimes;
}
