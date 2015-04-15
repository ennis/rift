#ifndef SCENE_HPP
#define SCENE_HPP

#include <gl4/renderer.hpp>
#include <hudtext.hpp>
#include <font.hpp>

// données partagées entre les shaders
struct SceneData
{
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	glm::mat4 viewProjMatrix;
	glm::vec4 eyePos;	// in world space
	glm::vec4 lightDir;
	glm::vec2 viewportSize;
};

struct SceneRenderContext
{
	RenderTarget *renderTarget;
	RenderTarget *overlayRenderTarget;
	// drawn before postFX (opaque)
	RenderQueue2 *opaqueRenderQueue;
	// drawn after postFX (HUD)
	RenderQueue2 *overlayRenderQueue;
	// TODO
	RenderQueue2 *shadowRenderQueue;
	HUDTextRenderer *textRenderer;
	Font *defaultFont;
	SceneData sceneData;
	BufferDesc sceneDataCB;
};

class SceneRenderer
{
public:
	// render: Mesh (VB/IB/layout/draw command) + Effect + EffectParameters 
	// SceneRenderer::doRender()
	//		iterate over all Renderables (component SceneRenderCache)
	//		call renderable->render(RenderQueue)
	//		
private:

};

 
#endif /* end of include guard: SCENE_HPP */