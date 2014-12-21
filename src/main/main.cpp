#include <game.hpp>
#include <string>
#include <transform.hpp>
#include <log.hpp>
#include <freecameracontrol.hpp>
#include <entity.hpp>
#include <renderer.hpp>
#include <AntTweakBar.h>
#include <sky.hpp>
#include <dds.hpp>
#include <terrain.hpp>
#include <shadersource.hpp>
#include <effect.hpp>
#include <font.hpp>
#include <hudtext.hpp>
#include <serialization.hpp>
#include <immediatecontext.hpp>
#include <model.hpp>
#include <skinnedmodelrenderer.hpp>
#include <animationclip.hpp>

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

	std::unique_ptr<EffectCompiler> effectCompiler;
	std::unique_ptr<HUDTextRenderer> hudTextRenderer;
	std::unique_ptr<ImmediateContextFactory> immediateContextFactory;
	std::unique_ptr<Model> model;
	std::unique_ptr<SkinnedModelRenderer> animTest;
	Pose testPose;
	//std::unique_ptr<AnimatedMesh> animTest;
	ImmediateContext *immediateContext;
	Font fnt;
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
	
	Renderer &rd = Engine::instance().getRenderer();
	
	effectCompiler = std::unique_ptr<EffectCompiler>(new EffectCompiler(&rd, "resources/shaders"));
	hudTextRenderer = std::unique_ptr<HUDTextRenderer>(new HUDTextRenderer(rd));
	immediateContextFactory = std::unique_ptr<ImmediateContextFactory>(new ImmediateContextFactory(rd));

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
	sky = new Sky(rd);
	sky->setTimeOfDay(21.0f);

	// terrain
	{
		Image heightmapData = Image::loadFromFile("resources/img/terrain/height.dds");
		assert(heightmapData.format() == ElementFormat::Unorm16);
		terrain = new Terrain(rd, std::move(heightmapData));
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
	fnt.loadFromFile(rd, "resources/img/fonts/arial.fnt");

	// test immediate context
	immediateContext = immediateContextFactory->create(200, PrimitiveType::Triangle);

	// test loading of animated mesh
	//animTest = std::unique_ptr<AnimatedMesh>(new AnimatedMesh(*immediateContextFactory, rd));
	//animTest->loadFromFile("resources/models/animated/mokou.dae");
	model = std::unique_ptr<Model>(new Model(rd, "resources/models/danbo/danbo.dae.mesh"));
	animTest = std::unique_ptr<SkinnedModelRenderer>(new SkinnedModelRenderer(rd, *immediateContextFactory, *model));

	// test loading of animation clips
	AnimationClip clip = AnimationClip::loadFromFile("resources/models/danbo/danbo@animation.anim");
	testPose = clip.computePose(0.1f);
}


void RiftGame::render(float dt)
{
	// rendu de la scene
	Renderer &R = Engine::instance().getRenderer();

	// render to screen
	R.bindRenderTargets(0, nullptr, nullptr);
	// update viewport
	glm::ivec2 win_size = Engine::instance().getWindow().size();
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
	pfsp.viewportSize = win_size;
	pfsp.viewProjMatrix = rc.projMatrix * rc.viewMatrix;
	R.updateBuffer(frameData, 0, sizeof(PerFrameShaderParameters), &pfsp);
	rc.pfsp = pfsp;

	// draw mesh
	meshPS->setup();

	// draw sky!
	//sky->render(rc);

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

	immediateContext->clear()
		.addVertex(Vertex({ 0, 0, 0 }, { 0.0, 0.7, 1.0, 1.0 }))
		.addVertex(Vertex({ 0, 1, 0 }, { 0.0, 0.7, 1.0, 1.0 }))
		.addVertex(Vertex({ 0, 1, 1 }, { 1.0, 0.7, 1.0, 1.0 }))
		.addVertex(Vertex({ 0, 1, 2 }, { 1.0, 0.7, 0.5, 1.0 }))
		.addVertex(Vertex({ 0, 2, 2 }, { 1.0, 0.7, 0.1, 1.0 }))
		.addVertex(Vertex({ 1, 2, 2 }, { 1.0, 0.5, 0.0, 1.0 }))
		.render(rc);

	//animTest->applyPose(testPose);
	animTest->draw(rc);

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

	sky->setTimeOfDay(twTimeOfDay);
}

void RiftGame::tearDown()
{
	TwDeleteAllBars();
	Entity::destroy(cameraEntity);
	Entity::destroy(light);
	delete terrain;
	delete sky;
}

int main()
{
	logInit("log");
	Window window("Rift", 1280, 720);
	Engine::run<RiftGame>(window);
	return 0;
}
