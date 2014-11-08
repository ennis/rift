#include <game.hpp>
#include <string>
#include <transform.hpp>
#include <log.hpp>
#include <freecameracontrol.hpp>
#include <yaml-cpp/yaml.h>
#include <entity.hpp>
#include <meshrenderable.hpp>
#include <renderer.hpp>
#include <mesh.hpp>
#include <AntTweakBar.h>
#include <sky.hpp>
#include <dds.hpp>
#include <terrain.hpp>
#include <sharedresources.hpp>
#include <shadersource.hpp>
#include <effect.hpp>
#include <font.hpp>
#include <hudtext.hpp>
#include <serialization.hpp>

//============================================================================
class RiftGame : public Game
{
public:
	RiftGame() : Game(glm::ivec2(1280, 720))
	{}

	void init();
	void render(float dt);
	void update(float dt);
	void tearDown();
	void initTweakBar();

private:
	float mLastTime = 0.f;
	float mTotalTime = 0.f;
	float mFPS = 0;

	std::unique_ptr<EffectCompiler> effectCompiler;
	std::unique_ptr<HUDTextRenderer> hudTextRenderer;
	Font fnt;
	Entity *buddha;
	Entity *cameraEntity;
	Entity *light;
	Entity *sprite;

	ConstantBuffer *frameData = nullptr;
	Terrain *terrain = nullptr;
	Sky *sky = nullptr;
	Effect testEffect;

	ShaderSource meshVS;
	ShaderSource meshFS;
	std::unique_ptr<Effect> meshEffect;
	PipelineState *meshPS;

	TwBar *tweakBar = nullptr;

	float twSunDirection[3];
	float twTimeOfDay;
};

//=============================================================================
void RiftGame::initTweakBar()
{
	tweakBar = TwNewBar("Settings");
	twSunDirection[0] = 0;
	twSunDirection[1] = -1;
	twSunDirection[2] = 0;
	TwAddVarRW(
		tweakBar,
		"SunDir",
		TW_TYPE_DIR3F,
		&twSunDirection,
		"label='Sun Direction'");
	twTimeOfDay = 21.f;
	TwAddVarRW(
		tweakBar,
		"TimeOfDay",
		TW_TYPE_FLOAT,
		&twTimeOfDay,
		"label='Time of day'");
}

//============================================================================
void RiftGame::init()
{
	initTweakBar();
	
	Renderer &rd = renderer();
	effectCompiler = std::unique_ptr<EffectCompiler>(new EffectCompiler(&rd, "resources/shaders"));
	hudTextRenderer = std::unique_ptr<HUDTextRenderer>(new HUDTextRenderer(rd));

	// create the buddha
	{
		buddha = Entity::create();
		// attach a mesh loaded from a file
		//auto meshRenderable = sceneManager->createMeshInstance(buddha, "resources/models/rock2.mesh");
		auto mesh = Mesh::loadFromFile(rd, "resources/models/rock2.mesh");
		buddha->addComponent<MeshRenderable>(rd)->setMesh(mesh);
		// set the material
		//meshRenderable->setMaterial(...);
		// move it to the center and scale it down
		buddha->getTransform().move(glm::vec3(200, 0, 200)).scale(0.1f);
	}

	// create the camera object
	{
		// create the camera entity (maybe not necessary...)
		cameraEntity = Entity::create();
		// create a camera object
		auto camera = cameraEntity->addComponent<Camera>();
		// center camera position
		cameraEntity->getTransform().move(glm::vec3(0, 0, -1));
		// add camera controller script
		cameraEntity->addComponent<FreeCameraController>();
	}

	// light
	{
		light = Entity::create();
		light->getTransform().move(glm::vec3(10, 10, 10));
	}


	frameData = rd.createConstantBuffer(sizeof(PerFrameShaderParameters), ResourceUsage::Dynamic, nullptr);

	// TEST TEST
	sky = new Sky();
	sky->setTimeOfDay(21.0f);


	// terrain
	{
		Image *heightmapData = new Image();
		heightmapData->loadFromFile("resources/img/terrain/tamrielheightsmall.dds");
		assert(heightmapData->format() == ElementFormat::Unorm16);
		terrain = new Terrain(rd, heightmapData);
	}

	// Effect test
	Shader *sh = rd.createShader(
		loadShaderSource("resources/shaders/sprite/vert.glsl").c_str(),
		loadShaderSource("resources/shaders/sprite/frag.glsl").c_str());

	auto ps1 = effectCompiler->createPipelineStateFromShader(
		&testEffect, sh);

	meshVS.loadFromFile("resources/shaders/mesh_default/vert.glsl", ShaderSourceType::Vertex);
	meshFS.loadFromFile("resources/shaders/mesh_default/frag.glsl", ShaderSourceType::Fragment);
	meshEffect = std::unique_ptr<Effect>(new Effect(&meshVS, &meshFS));
	// create pipeline state
	meshPS = effectCompiler->createPipelineState(meshEffect.get());

	// font loading
	fnt.loadFromFile(rd, "resources/img/fonts/arno_pro.fnt");

	// test serialization
	std::ofstream fileOut("test.bin", std::ios::binary | std::ios::out);
	BinaryTag::Writer testBT(fileOut);
	testBT.beginCompound("root");
	testBT.writeFloat("scaleX", 1.0f);
	testBT.writeFloat("scaleY", 2.0f);
	testBT.writeFloat("scaleZ", 3.0f);
	testBT.writeInt("int", -1);
	testBT.writeUint("uint", 2);
	testBT.beginCompound("rotation");
	testBT.writeString("path", "this/is/a/path");
	testBT.endCompound();
	testBT.endCompound();
}


void RiftGame::render(float dt)
{
	// rendu de la scene
	Renderer &R = renderer();

	// render to screen
	R.bindRenderTargets(0, nullptr, nullptr);
	// update viewport
	glm::ivec2 win_size = Game::getSize();
	Viewport vp{ 0.f, 0.f, float(win_size.x), float(win_size.y) };
	Viewport vp_half{ 0.f, 0.f, float(win_size.x/2), float(win_size.y/2) };
	R.setViewports(1, &vp);
	// clear RT
	R.clearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	R.clearDepth(100.f);

	// render context
	RenderContext rc;
	auto cam = cameraEntity->getComponent<Camera>();
	rc.viewMatrix = cam->getViewMatrix();
	rc.projMatrix = cam->getProjectionMatrix();
	rc.renderer = &R;
	rc.camera = cam;
	rc.perFrameShaderParameters = frameData;
	rc.renderPass = RenderPass::Opaque;

	// update constant buffer
	PerFrameShaderParameters pfsp;
	pfsp.eyePos = glm::vec4(cameraEntity->getTransform().position, 0.0f);
	pfsp.lightDir = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	pfsp.projMatrix = rc.projMatrix;
	pfsp.viewMatrix = rc.viewMatrix;
	pfsp.viewportSize = Game::getSize();
	pfsp.viewProjMatrix = rc.projMatrix * rc.viewMatrix;
	R.updateBuffer(frameData, 0, sizeof(PerFrameShaderParameters), &pfsp);
	rc.pfsp = pfsp;

	// draw mesh
	meshPS->setup();
	buddha->getComponent<MeshRenderable>()->render(rc);

	// draw sky!
	sky->render(rc);

	// draw terrain!
	terrain->render(rc);

	// draw stuff!
	/*hudTextRenderer->renderString(
		rc, 
		"Hello world!", 
		&fnt, 
		glm::vec2(200,200), 
		glm::vec4(1.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));*/
	
	hudTextRenderer->renderString(
		rc, 
		std::to_string(mFPS).c_str(), 
		&fnt, 
		glm::vec2(0,0), 
		glm::vec4(1.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	// render tweak bar
	//TwDraw();
}

void RiftGame::update(float dt)
{
	mLastTime += dt;
	mTotalTime += dt;

	// mise � jour du compteur FPS toutes les secondes
	if (mLastTime > 1.f) {
		mLastTime = 0.f;
		mFPS = 1.f / dt;
		glfwSetWindowTitle(Game::window(), ("Rift (" + std::to_string(mFPS) + " FPS)").c_str());
	}

	sky->setTimeOfDay(twTimeOfDay);
}

void RiftGame::tearDown()
{
	TwDeleteAllBars();
	Entity::destroy(cameraEntity);
	Entity::destroy(light);
	Entity::destroy(buddha);
	delete terrain;
	delete sky;
}

int main()
{
	logInit("log");
	Game::run(std::unique_ptr<Game>(new RiftGame()));
	return 0;
}
