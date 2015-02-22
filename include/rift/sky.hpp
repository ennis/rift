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

	float timeOfDay;
	Effect skyEffect;	
	BaseParameter CBParams;
	BaseParameter CBSceneData;
	Mesh skybox;
};

#endif