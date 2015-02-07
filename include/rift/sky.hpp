#ifndef SKY_HPP
#define SKY_HPP

#include <renderer2.hpp>
#include <renderable.hpp>
#include <effect.hpp>
#include <mesh.hpp>
#include <material.hpp>
#include <uniform.hpp>

class Sky : public Renderable
{
public:
	Sky(Renderer &renderer_);
	~Sky();

	void setTimeOfDay(float hour);
	void render(
		const RenderQueues &renderQueues, 
		const SceneData &data) override;

private:
	struct SkyParams
	{
		glm::vec3 sunDirection;
		glm::vec3 color;
	};

	Renderer *renderer;
	float timeOfDay;
	Effect skyEffect;
	Shader *skyShader;
	
	UniformValue<SkyParams> params;
	UniformBuffer<SceneData> sceneParams;
	
	Mesh skybox;
};

#endif