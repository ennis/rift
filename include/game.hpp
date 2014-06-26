#ifndef GAME_HPP
#define GAME_HPP

#include <common.hpp>
#include <renderer.hpp>
#include <opengl.hpp>
#include <resourcemanager.hpp>

//==========================================
// classe Game
//
class Game
{
public:
	Game(glm::ivec2 const &windowSize);
	virtual ~Game();

	//
	// M�thode appel�e lors de l'initialisation 
	virtual void init() = 0;

	//
	// M�thode appel�e au moment du rendu
	virtual void render(float dt) = 0;

	//
	// M�thode appel�e apr�s le rendu
	// fusionner avec render?
	virtual void update(float dt) = 0;

private:
	int run();
	int initContext();

public:
	//
	// Retourne la taille de la fen�tre
	static glm::ivec2 getSize() {
		return sWindowSize;
	}

	//
	// Retourne l'instance du renderer
	static CRenderer &renderer() {
		return *sRenderer;
	}

	//
	// retourne le pointeur vers la fen�tre GLFW
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
	static GLFWwindow *sWindow;
	static glm::ivec2 sWindowSize;
	static std::unique_ptr<Game> sGameInstance;
	static std::unique_ptr<CRenderer> sRenderer;
};

#endif