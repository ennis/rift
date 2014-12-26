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
#include <effect.hpp>
#include <font.hpp>
#include <hudtext.hpp>
#include <serialization.hpp>
#include <immediatecontext.hpp>
#include <model.hpp>
#include <skinnedmodelrenderer.hpp>
#include <animationclip.hpp>
#include <modelrenderer.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>

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

	std::unique_ptr<HUDTextRenderer> hudTextRenderer;
	std::unique_ptr<ImmediateContextFactory> immediateContextFactory;

	Effect eff;
	Material material;
	Model model;
	ModelRenderer staticModelRenderer;

	SkinnedModelRenderer animTest;
	Pose testPose;
	//std::unique_ptr<AnimatedMesh> animTest;
	ImmediateContext *immediateContext;
	Font fnt;
	Entity *cameraEntity;
	Entity *light;
	Entity *sprite;

	ConstantBuffer *frameData = nullptr;
	// Terrain class is non-nullable (non default-constructible), but we cannot 
	// initialize it in the member initialization list (we need to load an image first, perform various things, etc.)
	// So we need to use a wrapper class that allows an object to be null
	// until we are ready to construct it => boost::optional (soon std::optional)
	// (this is absolutely disgusting)
	// TODO maybe allow null-states / two-stage initialization
	boost::optional<Terrain> terrain;
	boost::optional<Sky> sky;

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
	boost::filesystem::path path(".");

	initTweakBar();
	
	Renderer &rd = Engine::instance().getRenderer();
	
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

	// need to use emplace for nonmoveable types
	sky.emplace(rd);
	sky->setTimeOfDay(21.0f);

	// terrain
	{
		Image heightmapData = Image::loadFromFile("resources/img/terrain/island.dds");
		assert(heightmapData.format() == ElementFormat::Unorm16);
		terrain.emplace(rd, std::move(heightmapData), nullptr, nullptr);
	}

	// Effect test
	Shader *sh = rd.createShader(
		loadShaderSource("resources/shaders/sprite/vert.glsl").c_str(),
		loadShaderSource("resources/shaders/sprite/frag.glsl").c_str());

	// load from file
	eff.loadFromFile("resources/shaders/default.glsl");

	// font loading
	fnt.loadFromFile(rd, "resources/img/fonts/arial.fnt");

	// test immediate context
	immediateContext = immediateContextFactory->create(200, PrimitiveType::Triangle);

	// test loading of animated mesh
	model = Model::loadFromFile(rd, "resources/models/animated/mokou.model");
	// create an optimized static mesh and send it to the GPU
	model.optimize();
	animTest = SkinnedModelRenderer(rd, model);
	staticModelRenderer.setModel(model);
	material.setEffect(eff);
	material.setParameter("eta", 2.0f);
	material.setParameter("shininess", 5.0f);
	material.setParameter("lightIntensity", 1.0f);
	staticModelRenderer.setMaterial(0, material);

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
	//pfsp.lightDir = glm::vec4(twSunDirection[0], twSunDirection[1], twSunDirection[2], 0.0f);
	pfsp.projMatrix = rc.projMatrix;
	pfsp.viewMatrix = rc.viewMatrix;
	pfsp.viewportSize = win_size;
	pfsp.viewProjMatrix = rc.projMatrix * rc.viewMatrix;
	R.updateBuffer(frameData, 0, sizeof(PerFrameShaderParameters), &pfsp);
	rc.pfsp = pfsp;

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

	/*auto ps = eff.compileShader(R, 0, nullptr);
	ps->setup(R); 
	R.setConstantBuffer(0, frameData);
	R.setNamedConstantFloat("eta", 2.0f);
	R.setNamedConstantFloat("shininess", 5.0f);
	R.setNamedConstantFloat("lightIntensity", 1.0f);
	R.setNamedConstantMatrix4("modelMatrix", Transform().scale(0.01).rotate(3.1415/2, glm::vec3(0,0,1)).toMatrix());
	const auto &mesh = model.getMesh();
	mesh.draw();*/

	staticModelRenderer.render(rc, Transform());

	//animTest->applyPose(testPose);
	//animTest.draw(rc);

	// render tweak bar
	TwDraw();
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
}

int main()
{
	logInit("log");
	Window window("Rift", 1280, 720);
	Engine::run<RiftGame>(window);
	return 0;
}
