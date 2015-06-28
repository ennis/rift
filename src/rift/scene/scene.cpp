#include <scene.hpp>
#include <colors.hpp>

namespace
{
	constexpr unsigned NumLastFrameTimes = 200;
}


Scene::Scene() :
lastEntityID(0),
lastFrameTimes(NumLastFrameTimes),
lastFrameIndex(0)
{
}

EntityID Scene::createEntity()
{
	auto id = lastEntityID++;
	entities.push_back(id);
	transforms.insert(std::make_pair(id, TransformNode()));
	return id;
}

EntityID Scene::createLightPrefab(const Transform &transform, LightMode lightMode, const glm::vec3 &intensity)
{
	auto id = createEntity();
	auto t = getTransformNode(id);
	t->transform = transform;
	auto light = CreateComponent(lightNodes, id);
	light->light.intensity = intensity;
	light->light.mode = lightMode;
	return id;
}

EntityID Scene::createMeshPrefab(const Transform &transform, Mesh &mesh, Material &material)
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

TransformNode *Scene::getTransformNode(EntityID id)
{
	return &transforms[id];
}

MeshNode *Scene::createMeshNode(EntityID id, Mesh &mesh, Material &material)
{
	auto m = CreateComponent(meshNodes, id);
	m->entity = id;
	m->material = &material;
	m->mesh = &mesh;
	return m;
}
	
LightNode *Scene::createPointLight(EntityID id, const glm::vec3 &intensity)
{
	auto l = CreateComponent(lightNodes, id);
	l->light.intensity = intensity;
	l->light.mode = LightMode::Point;
	return l;
}

LightNode *Scene::createDirectionalLight(EntityID id, const glm::vec3 &intensity)
{
	auto l = CreateComponent(lightNodes, id);
	l->light.intensity = intensity;
	l->light.mode = LightMode::Directional;
	return l;
}

util::array_ref<EntityID> Scene::getEntities() const
{
	return util::make_array_ref(entities);
}

void Scene::deleteEntity(EntityID id)
{
	LOG << "deleteEntity: TODO";
}
