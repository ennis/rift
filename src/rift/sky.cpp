#include <sky.hpp>
#include <gl4/effect.hpp>

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

	auto effect = gl4::Effect::loadFromFile("resources/shaders/sky.glsl");
	skyShader = effect->compileShader();

	cbSkyParams = Stream::create(BufferUsage::ConstantBuffer, sizeof(SkyParams), 3);

	skybox = Mesh::create(
		{ Attribute{ ElementFormat::Float3 } },
		36, 
		skycubeVertices, 
		0, 
		nullptr,
		{ Submesh{PrimitiveType::Triangle, 0, 0, 36, 0} }
	);
}


void Sky::setTimeOfDay(float hour)
{
	timeOfDay = hour;
}

void Sky::render(
	SceneRenderContext &context
	)
{
	SkyParams params;
	using namespace glm;
	float sunAngle = timeOfDay / 24.0f * 2 * 3.14159f;
	params.sunDirection = vec3(cosf(sunAngle), sinf(sunAngle), 0);
	params.sunColor = vec3(1.0f, 1.0f, 1.0f);
	cbSkyParams->write(params);

	auto &renderQueue = *context.opaqueRenderQueue;
	renderQueue.beginCommand();
	renderQueue.setUniformBuffers({ context.sceneDataCB, cbSkyParams->getDescriptor() });
	renderQueue.setShader(*skyShader);
	skybox->draw(renderQueue, 0);
}

