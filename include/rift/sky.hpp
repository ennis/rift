#ifndef SKY_HPP
#define SKY_HPP

//#include <gl4/renderer.hpp>
#include <scene.hpp>
#include <mesh.hpp>

class Sky
{
public:
	Sky();

	// VS2013
	Sky(Sky &&rhs) : 
		timeOfDay(rhs.timeOfDay), 
		skyPS(std::move(rhs.skyPS)),
		skybox(std::move(rhs.skybox))
	{}
	Sky &operator=(Sky &&rhs) {
		timeOfDay = rhs.timeOfDay;
		skyPS = std::move(rhs.skyPS);
		skybox = std::move(rhs.skybox);
		return *this;
	}
	// -VS2013

	void setTimeOfDay(float hour);
	void render(SceneRenderContext &context);

private:
	float timeOfDay;
	PipelineState::Ptr skyPS;
	Mesh::Ptr skybox;
};

#endif