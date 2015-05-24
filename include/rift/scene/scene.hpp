#ifndef SCENE_HPP
#define SCENE_HPP

#include <rendering/opengl4/graphics_context.hpp>
#include <rendering/opengl4/mesh_renderer.hpp>
#include <rendering/opengl4/text_renderer.hpp>
#include <rendering/opengl4/material.hpp>
#include <rendering/opengl4/light.hpp>
#include <mesh_data.hpp>
#include <scene/entity.hpp>
#include <resource_loader.hpp>
#include <font.hpp>
#include <transform.hpp>
#include <camera.hpp>

struct MeshNode
{
	Entity entity;
	gl4::Mesh *mesh;
	gl4::Material *material;
};

struct LightNode
{
	gl4::Light light;
};

struct TransformNode
{
	Entity parent = -1;
	Transform transform;
};

struct NVGcontext;

class Scene
{
public:
	Scene(gl4::GraphicsContext &graphicsContext_, ResourceLoader &resourceLoader);

	Entity createLightPrefab(const Transform &transform, gl4::LightMode lightMode, const glm::vec3 &intensity);
	Entity createMeshPrefab(const Transform &transform, gl4::Mesh &mesh, gl4::Material &material);

	Entity createEntity();
	LightNode *createPointLight(Entity id, const glm::vec3 &intensity);
	LightNode *createDirectionalLight(Entity id, const glm::vec3 &intensity);
	TransformNode *getTransformNode(Entity id);
	MeshNode *createMeshNode(Entity id, gl4::Mesh &mesh, gl4::Material &material);
	// LightNode *createLight(Entity id);
	void deleteEntity(Entity id);

	// debug message
	void render(Camera &camera, glm::ivec2 viewportSize, float dt);
	util::array_ref<Entity> getEntities() const;
	void loadFromFile(const char *path);

private:
	gl4::GraphicsContext &graphicsContext;
	// World (entity list)
	Entity lastEntity;
	std::vector<Entity> entities;
	EntityMap<TransformNode> transforms;
	EntityMap<glm::mat4> flattenedTransforms;
	EntityMap<MeshNode> meshNodes;
	EntityMap<LightNode> lightNodes;

	gl4::MeshRenderer meshRenderer;
	gl4::TextRenderer textRenderer;
	gl4::Material::Ptr defaultMaterial;
	Font::Ptr debugFont;

	NVGcontext *nvgContext;
	std::vector<float> lastFrameTimes;
	unsigned lastFrameIndex;

	// container for resources loaded by loadFromFile
	AssetDatabase assetDb;

	// Lights:

	// Feature renderers:
	// StaticMeshRenderer::Ptr staticMeshRenderer;
	// TerrainRenderer::Ptr terrainRenderer;
	// SkinnedMeshRenderer::Ptr skinnedMeshRenderer;
	// TextRenderer::Ptr textRenderer;

	// Postproc chain:
};

#endif /* end of include guard: SCENE_HPP */
