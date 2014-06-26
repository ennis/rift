#include <game.hpp>
#include <string>
#include <transform.hpp>
#include <log.hpp>
#include <freecameracontrol.hpp>
#include <modelloader.hpp>

//============================================================================
// Test test test
//
//
struct Bullet
{
	Transform transform;

	void render()
	{
	}
};

class RiftGame : public Game
{
public:
	RiftGame() : Game(glm::ivec2(800, 600))
	{}

	void init();
	void render(float dt);
	void update(float dt);

private:
	float mLastTime = 0.f;
	float mTotalTime = 0.f;
	CMesh *cubeMesh = nullptr;
	CTexture *texture = nullptr;
	CModel *buddha = nullptr;
	Transform meshPosition;

	Transform camPosition;
	Camera cam;
	FreeCameraControl camControl;
};

//============================================================================
// Le code du jeu à proprement parler
//
//
void RiftGame::init()
{
	// test cube
	static float cubeVertices[] = {
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f
	};

	static uint16_t cubeIndices[] = {
		0, 1, 2, 2, 3, 0,
		3, 2, 6, 6, 7, 3,
		7, 6, 5, 5, 4, 7,
		4, 0, 3, 3, 7, 4,
		0, 5, 1, 5, 0, 4,
		1, 5, 6, 6, 2, 1
	};

	MeshBufferInit cubeMbi;
	cubeMbi.desc.numVertices = 8;
	cubeMbi.desc.numIndices = 36;
	cubeMbi.desc.layoutType = Layout_Full;
	cubeMbi.desc.primitiveType = PrimitiveType::Triangles;
	cubeMbi.positions = cubeVertices;
	cubeMbi.indices = cubeIndices;
	cubeMesh = renderer().createMesh(cubeMbi);

	// Test texture
	TextureDesc td;
	td.format = PixelFormat::R8G8B8A8;
	td.size = glm::ivec3(800, 600, 1);
	td.textureType = TextureType::Texture2D;
	td.numMipMapLevels = 0;
	texture = renderer().createTexture(td);

	// load model
	buddha = loadModelFromOBJ(renderer(), "resources/models/buddha.obj");

	auto tex2 = textureManager().load("resources/img/mb_rocklface07_d.jpg");
	textureManager().printResources();
}


void RiftGame::render(float dt)
{
	auto &rd = renderer();

	// mise à jour de la camera
	camControl.update(cam, camPosition, dt);
	rd.setCamera(cam);

	// ici: rendu des objets
	cubeMesh->render(meshPosition);
	buddha->render(meshPosition);
	rd.render();
}

void RiftGame::update(float dt)
{
	mLastTime += dt;
	mTotalTime += dt;

	// mise à jour du compteur FPS toutes les secondes
	if (mLastTime > 1.f) {
		mLastTime = 0.f;
		glfwSetWindowTitle(Game::window(), ("Rift (" + std::to_string(1.f / dt) + " FPS)").c_str());
	}
}


int main()
{
	logInit("log");
	Game::run(std::unique_ptr<Game>(new RiftGame()));
	return 0;
}