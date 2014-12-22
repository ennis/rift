#include <game.hpp>	// class Game
#include <transform.hpp>	// struct Transform
#include <log.hpp>
#include <freecameracontrol.hpp>	// component FreeCameraController
#include <entity.hpp>	// class Entity
#include <renderer.hpp>	// class Renderer
#include <shadersource.hpp>	// loadShaderSource
#include <mesh.hpp>		// class Mesh
#include <renderable.hpp>	// struct PerFrameShaderParameters
#include <image.hpp> // class Image

#include <string>

//============================================================================
class TestImage : public Game
{
public:
	TestImage()
	{
		init();
	}

	void init();
	void render(float dt);
	void update(float dt);
	void tearDown();

private:
	float mLastTime = 0.f;
	float mTotalTime = 0.f;
	float mFPS = 0;

	// l'image à charger
	Image image;

};

//============================================================================
void TestImage::init()
{
	// Chargement d'une image depuis un fichier
	image = Image::loadFromFile("resources/img/default.tga");
	// test
	assert(image.getNumMipLevels() == 1);
	assert(image.getSize() == glm::ivec2(512, 512));
	// Unorm8x3 == 3 elements de type Unorm8 (i.e. un entier de 8 bits représentant une valeur entre 0.0 (0) et 1.0 (255) )
	// (aka RGB8, R8G8B8, etc...)
	assert(image.format() == ElementFormat::Unorm8x3);
}

void TestImage::render(float dt)
{
	// rendu de la scene
	Renderer &R = Engine::instance().getRenderer();

	// on dessine à l'écran
	R.bindRenderTargets(0, nullptr, nullptr);
	// on met à jour la taille du viewport opengl (pour qu'il corresponde à la taille de la fenêtre)
	glm::ivec2 win_size = Engine::instance().getWindow().size();
	Viewport vp{ 0.f, 0.f, float(win_size.x), float(win_size.y) };
	R.setViewports(1, &vp);
	// on efface le color buffer
	R.clearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	// on efface le depth buffer
	R.clearDepth(100.f);

	// rien d'autre à faire...
}

void TestImage::update(float dt)
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

void TestImage::tearDown()
{
}

int main()
{
	logInit("test_image");
	// fenêtre principale
	Window window("Rift", 1280, 720);
	// Création de la classe principale 
	// et démarrage de la boucle principale 
	Engine::run<TestImage>(window);
	return 0;
}
