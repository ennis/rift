#include <game.hpp>
#include <string>
#include <transform.hpp>
#include <log.hpp>
#include <AntTweakBar.h>
#include <serialization.hpp>
#include <scene.hpp>
#include <image.hpp>
#include <font.hpp>
#include <colors.hpp>
#include <resource_loader.hpp>
#include <mesh_data.hpp>
#include <camera.hpp>
#include <rendering/opengl4/graphics_context.hpp>
#include <rendering/opengl4/shader_compiler.hpp>
//#include <rendering/opengl4/text_renderer.hpp>
#define NANOVG_GL3_IMPLEMENTATION
#include <rendering/opengl4/nanovg/nanovg.h>
#include <rendering/opengl4/nanovg/nanovg_gl.h>

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
	void renderDebugHud();

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
	std::unique_ptr<TrackballCameraControl> trackball;
	gl4::Mesh *mokou;
	std::unique_ptr<Scene> scene;
	Entity cubeId;
	std::unique_ptr<gl4::GraphicsContext> graphicsContext;

	gl4::Shader *shader;
	gl4::Material::Ptr mat;

	ResourceLoader loader;
};

//============================================================================
void RiftGame::init()
{	
	graphicsContext = std::make_unique<gl4::GraphicsContext>();
	scene = std::make_unique<Scene>(*graphicsContext, loader);
	trackball = std::make_unique<TrackballCameraControl>(
		Engine::instance().getWindow(),
		glm::vec3{ 0.0f, 0.0f, -5.0f },
		45.0f,
		0.1,
		1000.0,
		0.01);

	mokou = loader.loadMesh("resources/models/mokou/mokou.mesh", *graphicsContext);
	mat = std::make_unique<gl4::Material>();
	mat->shader = loader.loadShader("resources/shaders/default.glsl", *graphicsContext);
	mat->diffuseMap = loader.loadTexture("resources/img/brick_wall.jpg");

	// test entities
	cubeId = scene->createMeshPrefab(Transform().scale(0.01f), *mokou, *mat);
	auto lightId = scene->createLightPrefab(
		Transform().move({ 0.f, 2.0f, 0.f }), 
		gl4::LightMode::Directional, 
		{ 0.0f, 1.0f, 0.8f });

	/*for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			auto id = scene->createMeshPrefab(Transform().scale(0.01f).move({ i*5.f, 0.f, j*5.f }), *mokou, *mat);
		}
	}*/

	// load scene from file
	scene->loadFromFile("resources/scenes/sample_scene/scene.bin", loader);
}

void RiftGame::render(float dt)
{
	graphicsContext->beginFrame();
	// rendu de la scene
	auto win_size = glm::ivec2(sizeX, sizeY);
	auto win_size_f = glm::vec2(sizeX, sizeY);
	auto cam = trackball->updateCamera();

	scene->render(cam, win_size, dt);

	Logging::screenMessage("F       : " + std::to_string(numFrames));
	Logging::screenMessage("DT      : " + std::to_string(dt));
	Logging::screenMessage("E       : " + std::to_string(scene->getEntities().size()));

	renderDebugHud();
	graphicsContext->endFrame();
}

void RiftGame::renderDebugHud()
{
	Logging::clearScreenMessages();
	/*CommandBuffer cmdBuf;
	auto lines = Logging::clearScreenMessages();
	unsigned ypos = 5;
	unsigned xpos = 5;
	unsigned yinc = debugFont->getMetrics().height;
	for (auto &&line : lines)
	{
		// drop shadow FTW
		hud->render(
			cmdBuf,
			line,
			*debugFont,
			glm::vec2(xpos+2, ypos+2),
			glm::vec2(sizeX, sizeY),
			Color::Black,
			glm::vec4(0.0, 0.0, 0.0, 0.0));
		hud->render(
			cmdBuf,
			line,
			*debugFont,
			glm::vec2(xpos, ypos),
			glm::vec2(sizeX, sizeY),
			Color::White,
			glm::vec4(0.0, 0.0, 0.0, 0.0));
		ypos += yinc;
	}
	graphicsContext->execute(cmdBuf);*/
}

void RiftGame::update(float dt)
{
	auto win_size = Engine::instance().getWindow().size();
	sizeX = win_size.x;
	sizeY = win_size.y;

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
	scene->deleteEntity(cubeId);
}

int main()
{
	Window window("Rift", 1280, 720);
	Engine::run<RiftGame>(window);
	return 0;
}
