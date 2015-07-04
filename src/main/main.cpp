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
	AssetDatabase assetDb;
	Mesh *mokou;
	EntityID cubeId;
	Shader *shader;
	Material::Ptr mat;
	EntityID sphereId;
	std::unique_ptr<BoundingCapsule> capsule_42;
	Coroutine cc;
};

void FiberTest2(float t)
{
	using namespace std::chrono;
	for (int sec = 0;;)
	{
		qpc_clock::time_point tstart = qpc_clock::now();
		yield(WaitForSeconds(1.0));
		qpc_clock::time_point tstop = qpc_clock::now();
		LOG << "Seconds: " << duration_cast<duration<double>>(tstop-tstart).count();
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

	// fiber test
	cc = Coroutine::start(FiberTest2, 0.52);
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
	Application app("Rift", 800, 480, options);
	app.run<RiftGame>();
	return 0;
}
