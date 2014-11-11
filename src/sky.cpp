#include <sky.hpp>
#include <game.hpp>

Sky::Sky() : mTimeOfDay(0.f), mSkybox(Game::renderer())
{
	auto &renderer = Game::renderer();
	mSkyShader = renderer.createShader(
		loadShaderSource("resources/shaders/sky/vert.glsl").c_str(),
		loadShaderSource("resources/shaders/sky/frag.glsl").c_str());

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

	ElementFormat layout[] = { ElementFormat::Float3 };
	mSkybox.allocate(PrimitiveType::Triangle, 1, layout, ResourceUsage::Static, 36, skycubeVertices);
}

Sky::~Sky()
{
	mSkyShader->release();
}

void Sky::setTimeOfDay(float hour)
{
	mTimeOfDay = hour;
}

void Sky::render(RenderContext const &context) 
{
	using namespace glm;
	float sunAngle = mTimeOfDay / 24.0f * 2 * 3.14159f;
	vec3 sunDirection = vec3(cosf(sunAngle), sinf(sunAngle), 0);
	context.renderer->setShader(mSkyShader);
	context.renderer->setConstantBuffer(0, context.perFrameShaderParameters);
	context.renderer->setNamedConstantFloat3("uSunDirection", sunDirection);
	context.renderer->setNamedConstantFloat3("uSunColor", glm::vec3(1.0f, 1.0f, 1.0f));
	mSkybox.draw();
}

