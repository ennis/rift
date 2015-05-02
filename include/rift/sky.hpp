#ifndef SKY_HPP
#define SKY_HPP

#include <gl4/renderer.hpp>
#include <scene.hpp>
#include <mesh.hpp>

class Sky
{
public:
	Sky();

	// VS2013
	Sky(Sky &&rhs) : 
		timeOfDay(rhs.timeOfDay), 
		skyShader(std::move(rhs.skyShader)),
		skybox(std::move(rhs.skybox))
	{}
	Sky &operator=(Sky &&rhs) {
		timeOfDay = rhs.timeOfDay;
		skyShader = std::move(rhs.skyShader);
		skybox = std::move(rhs.skybox);
		return *this;
	}
	// -VS2013

	void setTimeOfDay(float hour);
	void render(SceneRenderContext &context);

private:
	float timeOfDay;
	Shader::Ptr skyShader;
	Mesh::Ptr skybox;
};

#endif