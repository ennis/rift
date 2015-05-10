#ifndef PASS_HPP
#define PASS_HPP

#include <rendering/opengl4/opengl4.hpp>
#include <rendering/opengl4/light.hpp>

namespace gl4
{
	struct SceneView
	{
		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
		glm::mat4 viewProjMatrix;
		glm::vec4 lightDir;
		glm::vec4 wEye;	// in world space
		glm::vec2 viewportSize;
	};

	struct ShadowPassContext
	{
		SceneView *sceneView;
		Buffer *sceneViewUBO;
	};

	struct ForwardPassContext
	{
		SceneView *sceneView;
		Buffer *sceneViewUBO;
		Light *light;
		Buffer *lightParamsUBO;
		CommandBuffer *cmdBuf;
		// TODO shadow maps?
	};
}

 
#endif /* end of include guard: PASS_HPP */