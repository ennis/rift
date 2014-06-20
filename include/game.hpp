#ifndef GAME_HPP
#define GAME_HPP

#include <common.hpp>
#include <renderer/renderer.hpp>


//==========================================
// classe Game
//
// Les sous-classes doivent implémenter les méthodes init, render et update
class Game
{
public:
	Game(glm::ivec2 const &windowSize);
	virtual ~Game();

	//
	// Méthode appelée lors de l'initialisation 
	void init();

	//
	// Méthode appelée au moment du rendu
	void render(float dt);

	//
	// Méthode appelée après le rendu
	// fusionner avec render?
	void update(float dt);

private:
	int run();
	int initContext();

public:
	//
	// Retourne la taille de la fenêtre
	static glm::ivec2 getSize() {
		return sWindowSize;
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
	static GLFWwindow *sWindow;
	static glm::ivec2 sWindowSize;
	static std::unique_ptr<Game> sGameInstance;
	static std::unique_ptr<Renderer> sRenderer;

	float mLastTime = 0.f;
	float mTotalTime = 0.f;
};

#endif