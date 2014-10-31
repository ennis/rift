#include <sky.hpp>
#include <game.hpp>

Sky::Sky() : mTimeOfDay(0.f)
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

	mSkyboxVB = renderer.createVertexBuffer(3*sizeof(float), 36, ResourceUsage::Static, skycubeVertices);

	VertexElement layout[] = {
		VertexElement(0, 0, 0, 3*sizeof(float), ElementFormat::Float3)
	};

	mSkyboxVBLayout = renderer.createVertexLayout(1, layout);
}

Sky::~Sky()
{
	mSkyboxVB->release();
	mSkyboxVBLayout->release();
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
	context.renderer->setVertexBuffer(0, mSkyboxVB);
	context.renderer->setIndexBuffer(nullptr);
	context.renderer->setVertexLayout(mSkyboxVBLayout);
	// draw the box
	context.renderer->draw(PrimitiveType::Triangle, 0, 36);
}

