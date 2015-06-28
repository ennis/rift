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
	unsigned long numFrames = 0;
	unsigned sizeX, sizeY;
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
	BoundingSphere *sphere_42;
};

//============================================================================
void RiftGame::initialize(Application &app)
{	
	application = &app;
	auto &graphicsContext = app.getGraphicsContext();
	unsigned width, height;
	application->getSize(width, height);
	scene = std::make_unique<Scene>();
	sceneRenderer = std::make_unique<SceneRenderer>(glm::ivec2(width, height), graphicsContext, assetDb);
	trackball = std::make_unique<TrackballCameraControl>(app, glm::vec3{ 0.0f, 0.0f, -5.0f }, 45.0f, 0.1, 1000.0, 0.01);
	// load scene from file
	scene->loadFromFile(graphicsContext, "resources/scenes/sample_scene/scene.bin");
	scene->createLightPrefab(Transform().move({ 0.0f, -50.0f, 0.0f }), LightMode::Directional, { 1.0f, 1.0f, 1.0f });
}

//============================================================================
void RiftGame::render(float dt)
{
	// rendu de la scene
	auto win_size = glm::ivec2(sizeX, sizeY);
	auto win_size_f = glm::vec2(sizeX, sizeY);
	auto cam = trackball->getCamera();
	sceneRenderer->renderScene(*scene, cam, dt);
	Logging::screenMessage("F       : " + std::to_string(numFrames));
	Logging::screenMessage("DT      : " + std::to_string(dt));
	Logging::screenMessage("E       : " + std::to_string(scene->getEntities().size()));
}

//============================================================================
void RiftGame::update(float dt)
{
	application->getSize(sizeX, sizeY);

	mLastTime += dt;
	mTotalTime += dt;

	// mise à jour du compteur FPS toutes les secondes
	if (mLastTime > 1.f) {
		mLastTime = 0.f;
		mFPS = 1.f / dt;
		application->setTitle(("Rift (" + std::to_string(mFPS) + " FPS)").c_str());
	}

	++numFrames;
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
