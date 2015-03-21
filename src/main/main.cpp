#include <game.hpp>
#include <string>
#include <transform.hpp>
#include <log.hpp>
#include <freecameracontrol.hpp>
#include <entity.hpp>
#include <AntTweakBar.h>
#include <serialization.hpp>
#include <animationclip.hpp>
#include <scene.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <image.hpp>
#include <sky.hpp>
#include <gl4/renderer.hpp>
#include <gl4/effect.hpp>	// Effect
#include <font.hpp>
#include <hudtext.hpp>
#include <colors.hpp>

//============================================================================
// Classe de base du jeu
class RiftGame : public Game
{
public:
	// Constructeur
	RiftGame()
	{
		init();
	}

	void init();
	void render(float dt);
	void update(float dt);
	void tearDown();
	void initTweakBar();

private:
	float mLastTime = 0.f;
	float mTotalTime = 0.f;
	float mFPS = 0;
	float spinAngle = 0.f;
	unsigned long numFrames;

	struct PerObject
	{
		glm::mat4 modelMatrix;
		glm::vec4 objectColor;
	} perObj;

	struct PerObjectPBR
	{
		glm::mat4 modelMatrix;
		glm::vec4 objectColor;
		float eta;
	} perObjPBR;

	struct EnvCubeParams
	{
		glm::mat4 modelMatrix;
	} envCubeParams;

	SceneData sceneData;

	Entity *cameraEntity;
	Mesh::Ptr mesh;
	Mesh::Ptr mokou;

	gl4::Effect::Ptr effect;
	gl4::Effect::Ptr effectEnvCube;
	gl4::Effect::Ptr effectPBR;

	Shader::Ptr shader;
	Shader::Ptr shaderWireframe;
	Shader::Ptr shaderPBR;
	Shader::Ptr shaderEnvCube;

	ParameterBlock::Ptr paramBlock;
	ParameterBlock::Ptr paramBlockPBR;
	ParameterBlock::Ptr paramBlockEnvCube;

	ConstantBuffer::Ptr cbSceneData;
	ConstantBuffer::Ptr cbPerObj;
	ConstantBuffer::Ptr cbPerObjPBR;
	ConstantBuffer::Ptr cbEnvCube;

	RenderQueue::Ptr renderQueue;
	Texture2D::Ptr tex;
	TextureCubeMap::Ptr envmap;
	
	Texture2D::Ptr shadowMap;
	RenderTarget::Ptr shadowRT;

	Font::Ptr font;
	std::unique_ptr<HUDTextRenderer> hud;

	Sky sky;
};

//============================================================================
void RiftGame::init()
{
	boost::filesystem::path path(".");

	// create the camera object
	
	// create the camera entity (maybe not necessary...)
	cameraEntity = Entity::create();
	// create a camera object
	auto camera = cameraEntity->addComponent<Camera>();
	// center camera position
	cameraEntity->getTransform().move(glm::vec3(0, 0, -1));
	// add camera controller script
	cameraEntity->addComponent<FreeCameraController>();

	// Effect 
	effect = gl4::Effect::loadFromFile("resources/shaders/default.glsl");
	shader = effect->compileShader();
	RasterizerDesc rs = {};
	rs.fillMode = PolygonFillMode::Wireframe;
	shaderWireframe = effect->compileShader({}, rs, DepthStencilDesc{});

	effectPBR = gl4::Effect::loadFromFile("resources/shaders/pbr.glsl");
	shaderPBR = effectPBR->compileShader();
	effectEnvCube = gl4::Effect::loadFromFile("resources/shaders/envcube.glsl");
	shaderEnvCube = effectEnvCube->compileShader();

	// buffer contenant les données des vertex (c'est un cube, pour info)
	// ici: position (x,y,z), normales (x,y,z), coordonnées de texture (x,y) 
	// les normales et les coordonnées de textures sont fausses (c'est juste pour illustrer)
	static float cubeMeshData[] = {
		/* pos */ 0.0f, 0.0f, 1.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 1.0f, 0.0f, 1.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 1.0f, 1.0f, 1.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 0.0f, 1.0f, 1.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 0.0f, 0.0f, 0.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 1.0f, 0.0f, 0.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 1.0f, 1.0f, 0.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 0.0f, 1.0f, 0.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
	};

	// les indices (données de l'index buffer)
	static uint16_t cubeIndices[] = {
		0, 1, 2, 2, 3, 0,
		3, 2, 6, 6, 7, 3,
		7, 6, 5, 5, 4, 7,
		4, 0, 3, 3, 7, 4,
		0, 5, 1, 5, 0, 4,
		1, 5, 6, 6, 2, 1
	};

	Attribute attribs[] = {
		{ ElementFormat::Float3, ResourceUsage::Static },
		{ ElementFormat::Float3, ResourceUsage::Static },
		{ ElementFormat::Float2, ResourceUsage::Static },
	};

	mesh = Mesh::create(
		PrimitiveType::Triangle,
		attribs,
		8,
		cubeMeshData,
		36,
		cubeIndices);

	std::ifstream mokou_file("resources/models/mokou/mokou.mesh", std::ios::binary);
	serialization::IArchive arc(mokou_file);
	mokou = Mesh::loadFromArchive(arc);

	tex = Image::loadFromFile("resources/img/brick_wall.jpg").convertToTexture2D();
	envmap = Image::loadFromFile("resources/img/env/uffizi/env.dds").convertToTextureCubeMap();

	cbSceneData = ConstantBuffer::create(sizeof(SceneData), nullptr);
	cbPerObj = ConstantBuffer::create(sizeof(PerObject), nullptr);
	cbPerObjPBR = ConstantBuffer::create(sizeof(PerObjectPBR), nullptr);
	cbEnvCube = ConstantBuffer::create(sizeof(EnvCubeParams), nullptr);

	paramBlock = ParameterBlock::create(*shader);
	paramBlockPBR = ParameterBlock::create(*shaderPBR);
	paramBlockEnvCube = ParameterBlock::create(*shaderEnvCube);
	renderQueue = RenderQueue::create();

	glm::ivec2 win_size = Engine::instance().getWindow().size();
	shadowMap = Texture2D::create(win_size, 1, ElementFormat::Depth16, nullptr);
	shadowRT = RenderTarget::createRenderTarget2D(*shadowMap, 0);

	font = Font::loadFromFile("resources/img/fonts/special_elite.fnt");
	hud = std::make_unique<HUDTextRenderer>();
}


void RiftGame::render(float dt)
{
	// rendu de la scene
	glm::ivec2 win_size = Engine::instance().getWindow().size();
	auto &R = Renderer::getInstance();

	R.setRenderTargets({}, nullptr);
	R.clearColor(0.25f, 0.25f, 0.2f, 0.0f);
	R.clearDepth(1000.f);
	R.setViewports({ { 0.f, 0.f, float(win_size.x), float(win_size.y), 0.0f, 1.0f } });

	// update scene data buffer
	auto cam = cameraEntity->getComponent<Camera>();
	sceneData.eyePos = glm::vec4(cameraEntity->getTransform().position, 1.0f);
	sceneData.projMatrix = cam->getProjectionMatrix();
	sceneData.viewMatrix = cam->getViewMatrix();
	sceneData.viewProjMatrix = sceneData.projMatrix * sceneData.viewMatrix;
	sceneData.viewportSize = win_size;
	cbSceneData->update(0, sizeof(SceneData), &sceneData);

	// update per-model buffer
	perObj.modelMatrix = glm::mat4(1.0f);
	perObj.objectColor = glm::vec4(1.0f);
	cbPerObj->update(0, sizeof(PerObject), &perObj);

	// update per-model buffer
	spinAngle = fmodf(spinAngle + 0.1f*3.14159f*dt, 2 * 3.14159);
	perObjPBR.modelMatrix = glm::rotate(glm::mat4(1.0f), spinAngle, glm::vec3{ 0, 1, 0 });
	perObjPBR.objectColor = glm::vec4(1.0f);
	perObjPBR.eta = 4.0f;
	cbPerObjPBR->update(0, sizeof(PerObjectPBR), &perObjPBR);

	envCubeParams.modelMatrix = glm::translate(glm::scale(glm::vec3{ 1000.0f, 1000.0f, 1000.0f }), glm::vec3{ -0.5f, -0.5f, -0.5f });
	cbEnvCube->update(0, sizeof(EnvCubeParams), &envCubeParams);

	// create parameter block
	paramBlock->setConstantBuffer(0, *cbSceneData);
	paramBlock->setConstantBuffer(1, *cbPerObj);
	paramBlock->setTextureParameter(0, tex.get(), SamplerDesc{});

	paramBlockPBR->setConstantBuffer(0, *cbSceneData);
	paramBlockPBR->setConstantBuffer(1, *cbPerObjPBR);
	paramBlockPBR->setTextureParameter(0, tex.get(), SamplerDesc{});
	paramBlockPBR->setTextureParameter(1, envmap.get(), SamplerDesc{});

	paramBlockEnvCube->setConstantBuffer(0, *cbSceneData);
	paramBlockEnvCube->setConstantBuffer(1, *cbEnvCube);
	paramBlockEnvCube->setTextureParameter(0, envmap.get(), SamplerDesc{});

	// submit to render queue
	if (glfwGetKey(Engine::instance().getWindow().getHandle(), GLFW_KEY_H)) {
		//renderQueue.debugPrint();
	}

	if (glfwGetKey(Engine::instance().getWindow().getHandle(), GLFW_KEY_W)) {
		// XXX not the same parameter block!
		//renderQueue->draw(*mesh, 0, *shaderWireframe, *paramBlock, 0);
	}
	else {
		//renderQueue->draw(*mesh, 0, *shader, *paramBlock, 0);
	}

	hud->renderString("Hello world!", *font, { 100.0, 100.0 }, Color::White, Color::Black, *renderQueue, sceneData, *cbSceneData);

	renderQueue->draw(*mesh, Submesh{ PrimitiveType::Triangle, 0, 0, 8, 36 }, *shaderEnvCube, *paramBlockEnvCube, 0);
	renderQueue->draw(*mokou, Submesh{ PrimitiveType::Triangle, 0, 0, 100, 100 }, *shaderPBR, *paramBlockPBR, 0);

	//sky.render(*renderQueue, sceneData, *cbSceneData);
	R.submitRenderQueue(*renderQueue);

	R.setRenderTargets({}, shadowRT.get());
	R.clearDepth(1000.f);
	R.setViewports({ { 0.f, 0.f, float(win_size.x), float(win_size.y), 0.0f, 1.0f } });
	// resubmit
	R.submitRenderQueue(*renderQueue);

	renderQueue->clear();

	// render tweak bar
	//TwDraw();
}

void RiftGame::update(float dt)
{
	mLastTime += dt;
	mTotalTime += dt;

	// mise à jour du compteur FPS toutes les secondes
	if (mLastTime > 1.f) {
		mLastTime = 0.f;
		mFPS = 1.f / dt;
		Engine::instance().getWindow().setTitle(("Rift (" + std::to_string(mFPS) + " FPS)").c_str());
	}

	++numFrames;
}

void RiftGame::tearDown()
{
	TwDeleteAllBars();
	Entity::destroy(cameraEntity);
}

int main()
{
	logInit("log");
	Window window("Rift", 1280, 720);
	Engine::run<RiftGame>(window);
	return 0;
}
