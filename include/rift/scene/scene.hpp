#ifndef SCENE_HPP
#define SCENE_HPP

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