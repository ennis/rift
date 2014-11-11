#ifndef GAME_HPP
#define GAME_HPP

#include <common.hpp>
#include <opengl.hpp>
#include <resourcemanager.hpp>

class Renderer;

//==========================================
// classe Game
//
class Game
{
public:
	Game(glm::ivec2 const &windowSize);
	virtual ~Game();

	//
	// Méthode appelée lors de l'initialisation 
	virtual void init() = 0;

	//
	// Méthode appelée au moment du rendu
	virtual void render(float dt) = 0;

	//
	// Méthode appelée après le rendu
	// fusionner avec render?
	virtual void update(float dt) = 0;

private:
	int run();
	int initContext();

public:
	//
	// Retourne la taille de la fenêtre
	static glm::ivec2 getSize() {
		return sWindowSize;
	}

	static bool windowResized() {
		return sWindowResized;
	}

	//
	// Retourne l'instance du renderer
	static Renderer &renderer() {
		return *sRenderer;
	}

	//
	// retourne le pointeur vers la fenêtre GLFW
	static GLFWwindow *window() {
		return sWindow;
	}

	//
	//
	static void run(std::unique_ptr<Game> instance);


	//
	// Retourne l'instance du jeu
	static Game &instance() {
		return *sGameInstance;
	}
	

private:
	// make these pointers static to avoid another level of indirection
	static bool sWindowResized;
	static GLFWwindow *sWindow;
	static glm::ivec2 sWindowSize;
	static std::unique_ptr<Game> sGameInstance;
	static std::unique_ptr<Renderer> sRenderer;
};

#endif