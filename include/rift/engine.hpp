#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <window.hpp>
#include <AntTweakBar.h>
#include <renderer.hpp>

class Game;

class Engine
{
public:
	Renderer &getRenderer() {
		return mRenderer;
	}

	Window &getWindow() {
		return mWindow;
	}

	Game &getGame() {
		assert(mGame);
		return *mGame;
	}

	template <typename TGame>
	static void run(Window &window)
	{
		Engine e(window);
		TGame game;
		e.run(game);
	}

	static Engine &instance() 
	{
		return *gEngine;
	}

private:
	Engine(Window &window);

	void run(Game &game);
	void init();
	void mainLoop();

	Renderer mRenderer;
	Window &mWindow;
	Game *mGame;
	TwBar *mMainBar;
	bool mWindowResized;

	static Engine *gEngine;
};
 
#endif /* end of include guard: ENGINE_HPP */