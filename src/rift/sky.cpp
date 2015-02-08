#include <sky.hpp>

Sky::Sky(Renderer &renderer, Buffer *cbSceneData) 
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

	skyEffect = Effect("resources/shaders/sky.glsl");
	skyShader = skyEffect.compileShader(renderer, {});
	params = ConstantValue<SkyParams>(*skyShader, "CBSkyParams");
	sceneParams = ConstantBuffer(*skyShader, "SceneData", cbSceneData);

	Mesh::Attribute attribs[] = { { 0, ElementFormat::Float3 } };
	Mesh::BufferDesc buffers[] = { { ResourceUsage::Static } };
	const void *init[] = { skycubeVertices };
	skybox = Mesh(PrimitiveType::Triangle, attribs, buffers, 36, init, 0, ElementFormat::Max, ResourceUsage::Static, nullptr);

	renderer.setShader(skyShader);
	params.bind(renderer);
	sceneParams.bind(renderer);
	skybox.draw(renderer);
	submission = renderer.createSubmission();
}

Sky::~Sky()
{
}

void Sky::setTimeOfDay(float hour)
{
	timeOfDay = hour;
}

void Sky::render(
	RenderQueue &rq, 
	const SceneData &sceneData
	)
{
	using namespace glm;
	float sunAngle = timeOfDay / 24.0f * 2 * 3.14159f;
	vec3 sunDirection = vec3(cosf(sunAngle), sinf(sunAngle), 0);
	vec3 sunColor = vec3(1.0f, 1.0f, 1.0f);

	params.update({ sunDirection, sunColor });
	rq.submit(submission, 0);
}

