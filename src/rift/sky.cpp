#include <sky.hpp>

Sky::Sky(Renderer &renderer) 
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
	CBParams = BaseParameter(skyEffect, "skyParams");
	CBSceneData = BaseParameter(skyEffect, "sceneData");

	Mesh::Attribute attribs[] = { { 0, ElementFormat::Float3 } };
	Mesh::BufferDesc buffers[] = { { ResourceUsage::Static } };
	const void *init[] = { skycubeVertices };
	skybox = Mesh(PrimitiveType::Triangle, attribs, buffers, 36, init, 0, ElementFormat::Max, ResourceUsage::Static, nullptr);
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
	struct SkyParams
	{
		glm::vec3 sunDirection;
		glm::vec3 color;
	} params;

	using namespace glm;
	float sunAngle = timeOfDay / 24.0f * 2 * 3.14159f;
	params.sunDirection = vec3(cosf(sunAngle), sinf(sunAngle), 0);
	params.sunColor = vec3(1.0f, 1.0f, 1.0f);

	ParameterBlock pb(skyEffect);
	pb.setParameter(CBParams, params);
	pb.setParameterBuffer(CBSceneData, ...);
	rq.draw(skybox, 0, skyEffect, pb, 0);
}

