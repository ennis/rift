#ifndef SCENE_HPP
#define SCENE_HPP

#include <gl4/renderer.hpp>
#include <hudtext.hpp>
#include <font.hpp>
#include <material.hpp>
#include <mesh.hpp>
#include <transform.hpp>


// données partagées entre les shaders
struct SceneData
{
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	glm::mat4 viewProjMatrix;
	glm::vec4 eyePos;	// in world space
	// directional light
	glm::vec4 lightDir;
	glm::vec2 viewportSize;
};

struct SceneRenderContext
{
	RenderTarget *renderTarget;
	RenderTarget *overlayRenderTarget;
	// drawn before postFX (opaque)
	CommandBuffer *opaqueList;
	HUDTextRenderer *textRenderer;
	Font *defaultFont;
	SceneData sceneData;
	Buffer *sceneDataCB;
};

// un objet de la scène
struct SceneObject
{
	Mesh *mesh;
	Material material;
	Transform modelToWorld;
};

#endif /* end of include guard: SCENE_HPP */