#include <sky.hpp>

namespace
{
	struct SkyParams
	{
		glm::vec3 sunDirection;
		glm::vec3 sunColor;
	};
}

Sky::Sky()
{
	static const float skycubeVertices[] = {
		-10.0f, 10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, 10.0f, -10.0f,
		-10.0f, 10.0f, -10.0f,

		-10.0f, -10.0f, 10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f, 10.0f, -10.0f,
		-10.0f, 10.0f, -10.0f,
		-10.0f, 10.0f, 10.0f,
		-10.0f, -10.0f, 10.0f,

		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,

		-10.0f, -10.0f, 10.0f,
		-10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, -10.0f, 10.0f,
		-10.0f, -10.0f, 10.0f,

		-10.0f, 10.0f, -10.0f,
		10.0f, 10.0f, -10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		-10.0f, 10.0f, 10.0f,
		-10.0f, 10.0f, -10.0f,

		-10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f, 10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f, 10.0f,
		10.0f, -10.0f, 10.0f
	};

	skyEffect = Effect::create(
		loadEffectSource("resources/shaders/sky.glsl").c_str(), 
		"resources/shaders", 
		RasterizerDesc{}, 
		DepthStencilDesc{});

	cbSkyParams = ConstantBuffer::create(sizeof(SkyParams), nullptr);

	skybox = Mesh::create(
		PrimitiveType::Triangle, 
		{ Attribute{ ElementFormat::Float3, ResourceUsage::Static } },
		36, 
		skycubeVertices, 
		0, 
		nullptr, 
		{ Submesh{ 0, 0, 36, 0 } }
	); 
	
	paramBlock = ParameterBlock::create(*skyEffect);
	paramBlock->setConstantBuffer(1, *cbSkyParams);
}


void Sky::setTimeOfDay(float hour)
{
	timeOfDay = hour;
}

void Sky::render(
	RenderQueue &rq, 
	const SceneData &sceneData,
	const ConstantBuffer &cbSceneData
	)
{
	SkyParams params;
	using namespace glm;
	float sunAngle = timeOfDay / 24.0f * 2 * 3.14159f;
	params.sunDirection = vec3(cosf(sunAngle), sinf(sunAngle), 0);
	params.sunColor = vec3(1.0f, 1.0f, 1.0f);
	cbSkyParams->update(0, sizeof(SkyParams), &params);
	paramBlock->setConstantBuffer(0, cbSceneData);
	rq.draw(*skybox, 0, *skyEffect, *paramBlock, 0);
}

