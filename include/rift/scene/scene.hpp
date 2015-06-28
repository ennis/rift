#ifndef SCENE_HPP
#define SCENE_HPP

#include <rendering/opengl4.hpp>
#include <mesh_data.hpp>
#include <scene/entity.hpp>
#include <font.hpp>
#include <transform.hpp>
#include <camera.hpp>

struct MeshNode
{
	EntityID entity;
	Mesh *mesh;
	Material *material;
};

struct LightNode
{
	Light light;
};

struct TransformNode
{
	EntityID parent = -1;
	Transform transform;
};

class Scene
{
public:
	Scene();

	EntityID createLightPrefab(const Transform &transform, LightMode lightMode, const glm::vec3 &intensity);
	EntityID createMeshPrefab(const Transform &transform, Mesh &mesh, Material &material);
	EntityID createEntity();
	LightNode *createPointLight(EntityID id, const glm::vec3 &intensity);
	LightNode *createDirectionalLight(EntityID id, const glm::vec3 &intensity);
	TransformNode *getTransformNode(EntityID id);
	MeshNode *createMeshNode(EntityID id, Mesh &mesh, Material &material);
	void deleteEntity(EntityID id);

	util::array_ref<EntityID> getEntities() const;

	// load scene for file
	void loadFromFile(GraphicsContext &gc, const char *path);

	EntityID lastEntityID = 0;
	std::vector<EntityID> entities;
	EntityMap<TransformNode> transforms;
	EntityMap<glm::mat4> flattenedTransforms;
	EntityMap<MeshNode> meshNodes;
	EntityMap<LightNode> lightNodes;
	// Contains the last render times (for FPS graph)
	std::vector<float> lastFrameTimes;
	unsigned lastFrameIndex;
	AssetDatabase assetDb;
};

#endif /* end of include guard: SCENE_HPP */
