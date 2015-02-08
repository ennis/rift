#ifndef SKY_HPP
#define SKY_HPP

#include <renderer2.hpp>
#include <scene.hpp>
#include <effect.hpp>
#include <mesh.hpp>
#include <uniform.hpp>
#include <renderqueue.hpp>

class Sky
{
public:
	Sky(Renderer &renderer, Buffer *cbSceneData);
	~Sky();

	void setTimeOfDay(float hour);
	void render(RenderQueue &rq, const SceneData &sceneData);

private:
	struct SkyParams
	{
		glm::vec3 sunDirection;
		glm::vec3 color;
	};

	int submission;
	float timeOfDay;
	Effect skyEffect;
	Shader *skyShader;
	
	ConstantValue<SkyParams> params;
	ConstantBuffer sceneParams;
	
	Mesh skybox;
};

#endif