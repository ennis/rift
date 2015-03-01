#ifndef SKY_HPP
#define SKY_HPP

#include <gl4/renderer.hpp>
#include <scene.hpp>

class Sky
{
public:
	Sky();

	// VS2013
	Sky(Sky &&rhs) : 
		timeOfDay(rhs.timeOfDay), 
		skyEffect(std::move(rhs.skyEffect)), 
		cbSkyParams(std::move(rhs.cbSkyParams)), 
		skybox(std::move(rhs.skybox))
	{}
	Sky &operator=(Sky &&rhs) {
		timeOfDay = rhs.timeOfDay;
		skyEffect = std::move(rhs.skyEffect);
		cbSkyParams = std::move(rhs.cbSkyParams);
		skybox = std::move(rhs.skybox);
		return *this;
	}
	// -VS2013

	void setTimeOfDay(float hour);
	void render(RenderQueue &rq, 
		const SceneData &sceneData,
		const ConstantBuffer &cbSceneData);

private:
	float timeOfDay;
	Effect::Ptr skyEffect;
	ConstantBuffer::Ptr cbSkyParams;
	ParameterBlock::Ptr paramBlock;
	Mesh::Ptr skybox;
};

#endif