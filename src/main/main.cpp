#include <application.hpp>
#include <string>
#include <transform.hpp>
#include <log.hpp>
#include <AntTweakBar.h>
#include <scene.hpp>
#include <image.hpp>
#include <font.hpp>
#include <colors.hpp>
#include <mesh_data.hpp>
#include <camera.hpp>
#include <rendering/scene_renderer.hpp>
#include <rendering/nanovg/nanovg.h>
#include <boundingsphere.hpp>
#include <boundingcapsule.hpp>
#include <thread>
#include <utils/coroutine.hpp>
#include <screen.hpp>
#include <input.hpp>
#include <time.hpp>

using namespace glm;


//============================================================================
// Classe de base du jeu
class RiftGame : public MainLoop
{
public:
	// Constructeur
	~RiftGame();
	void initialize(Application &app) override;
	void render(float dt) override;
	void update(float dt) override;
	void initTweakBar();
private:
	float mLastTime = 0.f;
	float mTotalTime = 0.f;
	float mFPS = 0;
	struct PerObject
	{
		glm::mat4 modelMatrix;
		glm::vec4 objectColor;
	};
	Application *application;
	std::unique_ptr<TrackballCameraControl> trackball;
	std::unique_ptr<Scene> scene;
	std::unique_ptr<SceneRenderer> sceneRenderer;
	std::unique_ptr<Terrain> terrain;
	AssetDatabase assetDb;
	Mesh *mokou;
	EntityID cubeId;
	Shader *shader;
	Material::Ptr mat;
	EntityID sphereId;
	std::unique_ptr<BoundingCapsule> capsule_42;
	Coroutine cc;

	Image heightmap;
	Texture2D *terrainGrassTex;
	Texture2D *terrainRockTex;
};

void coroutineTest()
{
	using namespace std::chrono;
	double x_axis = 0.0;
	double y_axis = 0.0;
	for (int sec = 0;;)
	{
		// execute every second
		yield();
		double x_axis_new = input::getAxis(input::Axis_X);
		double y_axis_new = input::getAxis(input::Axis_Y);
		if (x_axis != x_axis_new || y_axis != y_axis_new) {
			LOG << "X:" << x_axis_new << " Y:" << y_axis_new;
			x_axis = x_axis_new;
			y_axis = y_axis_new;
		}
	}
}

//============================================================================
void RiftGame::initialize(Application &app)
{	
	application = &app;
	auto &graphicsContext = app.getGraphicsContext();
	int width = app.getWidth();
	int height = app.getHeight();
	scene = std::make_unique<Scene>();
	sceneRenderer = std::make_unique<SceneRenderer>(glm::ivec2(width, height), graphicsContext, assetDb);
	trackball = std::make_unique<TrackballCameraControl>(app, glm::vec3{ 0.0f, 0.0f, -5.0f }, 45.0f, 0.1, 1000.0, 0.01);
	// load scene from file
	scene->loadFromFile(graphicsContext, "resources/scenes/sample_scene/scene.bin");
	scene->createLightPrefab(Transform().move({ 0.0f, -50.0f, 0.0f }), LightMode::Directional, { 1.0f, 1.0f, 1.0f });

	// test bounding volumes
	capsule_42 = std::make_unique<BoundingCapsule>(glm::vec3{ 0, 0, 0 }, 2.0f, 8.0f, graphicsContext);

	// test terrain
	heightmap = Image::loadFromFile("resources/img/terrain/height.dds");
	TerrainInit init;
	init.heightmap = &heightmap;
	init.verticalScale = 50.f;
	init.flatTexture = loadTexture2DAsset(assetDb, graphicsContext, "resources/img/grasstile_c.jpg");
	init.slopeTexture = loadTexture2DAsset(assetDb, graphicsContext, "resources/img/rock.jpg");
	init.flatTextureScale = 1.0f;
	init.slopeTextureScale = 1.0f;
	terrain = createTerrain(graphicsContext, init);
	scene->terrain = terrain.get();

	cc = Coroutine::start(coroutineTest);
}

//============================================================================
void RiftGame::render(float dt)
{
	int width = application->getWidth();
	int height = application->getHeight();
	// rendu de la scene
	auto win_size = glm::ivec2(width, height);
	auto win_size_f = glm::vec2(width, height);
	auto cam = trackball->getCamera();
	sceneRenderer->setSceneCamera(cam);
	sceneRenderer->renderScene(*scene, dt);
	capsule_42->render(*sceneRenderer);
	Logging::screenMessage("F       : " + std::to_string(application->getFrameCount()));
	Logging::screenMessage("DT      : " + std::to_string(dt));
	Logging::screenMessage("E       : " + std::to_string(scene->getEntities().size()));
}

//============================================================================
void RiftGame::update(float dt)
{
	cc.resume();

	mLastTime += dt;
	mTotalTime += dt;

	// mise à jour du compteur FPS toutes les secondes
	if (mLastTime > 1.f) {
		mLastTime = 0.f;
		mFPS = 1.f / dt;
		application->setTitle(("Rift (" + std::to_string(mFPS) + " FPS)").c_str());
	}
}

RiftGame::~RiftGame()
{
	TwDeleteAllBars();
	scene->deleteEntity(cubeId);
}

int main()
{
	ContextOptions options;
	options.fullscreen = false;
	options.glMajor = 4;
	options.glMinor = 4;
	options.numSamples = 0;
	Application app("Rift", 1280, 720, options);
	app.run<RiftGame>();
	return 0;
}
