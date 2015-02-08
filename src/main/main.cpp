#include <game.hpp>
#include <string>
#include <transform.hpp>
#include <log.hpp>
#include <freecameracontrol.hpp>
#include <entity.hpp>
#include <renderer2.hpp>
#include <AntTweakBar.h>
#include <effect.hpp>
#include <serialization.hpp>
#include <model.hpp>
#include <renderqueue.hpp>
#include <animationclip.hpp>
#include <scene.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <uniform.hpp>
#include <sky.hpp>

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

	struct PerObject
	{
		glm::mat4 modelMatrix;
		glm::vec4 objectColor;
	} perObj;

	SceneData sceneData;

	Entity *cameraEntity;
	Shader *shader;
	ConstantValue<SceneData> CBSceneData;
	ConstantValue<PerObject> CBPerObj;
	std::vector<unsigned int> subs;
	Effect effect;
	Model model;
	std::unique_ptr<RenderQueue> rq;
	std::unique_ptr<Sky> sky;
};

//============================================================================
void RiftGame::init()
{
	boost::filesystem::path path(".");
	Renderer &R = Engine::instance().getRenderer();

	// create the camera object
	
	// create the camera entity (maybe not necessary...)
	cameraEntity = Entity::create();
	// create a camera object
	auto camera = cameraEntity->addComponent<Camera>();
	// center camera position
	cameraEntity->getTransform().move(glm::vec3(0, 0, -1));
	// add camera controller script
	cameraEntity->addComponent<FreeCameraController>();

	// Effect test
	// load from file
	effect = Effect("resources/shaders/default.glsl");
	shader = effect.compileShader(R, { { "USE_SHADOWS", "" } });
	// test loading of animated mesh
	model = Model(R, "resources/models/post.model");
	// create an optimized static mesh and send it to the GPU
	model.optimize();

	CBSceneData = ConstantValue<SceneData>(*shader, "SceneData");
	CBPerObj = ConstantValue<PerObject>(*shader, "PerObject");

	// create submission for each submesh
	rq = std::make_unique<RenderQueue>(R);
	rq->clearColor(0, 0.25f, 0.25f, 0.2f, 0.0f);
	rq->clearDepth(0, 1000.f);
	rq->setRenderTargets(0, { R.getScreenRenderTarget() }, R.getScreenDepthRenderTarget());
	const auto &mesh = model.getMesh();
	const auto &sm = model.getSubmeshes();
	for (const auto &s : sm) {
		R.setShader(shader);
		CBSceneData.bind(R);
		CBPerObj.bind(R);
		mesh.drawPart(R, s.startVertex, s.startIndex, s.numIndices);
		subs.push_back(R.createSubmission());
	}

	sky = std::make_unique<Sky>(R, CBSceneData.getBuffer());
}


void RiftGame::render(float dt)
{
	// rendu de la scene
	Renderer &R = Engine::instance().getRenderer();
	glm::ivec2 win_size = Engine::instance().getWindow().size();
	rq->setViewports(0, { { 0.f, 0.f, float(win_size.x), float(win_size.y) } });

	// update scene data buffer
	auto cam = cameraEntity->getComponent<Camera>();
	sceneData.eyePos = glm::vec4(cameraEntity->getTransform().position, 1.0f);
	sceneData.projMatrix = cam->getProjectionMatrix();
	sceneData.viewMatrix = cam->getViewMatrix();
	sceneData.viewProjMatrix = sceneData.projMatrix * sceneData.viewMatrix;
	sceneData.viewportSize = win_size;
	CBSceneData.update(sceneData);

	// update per-model buffer
	perObj.modelMatrix = glm::mat4(1.0f);
	perObj.objectColor = glm::vec4(1.0f);
	CBPerObj.update(perObj);

	for (auto sub : subs)
		rq->submit(sub, 0);
	rq->flush();

	// render sky
	sky->render(R, sceneData);

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
