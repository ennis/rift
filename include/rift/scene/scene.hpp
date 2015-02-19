#ifndef SCENE_HPP
#define SCENE_HPP

#include <buffer.hpp>

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


 
#endif /* end of include guard: SCENE_HPP */