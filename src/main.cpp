#include <game.hpp>

int main()
{
	Game::run(std::unique_ptr<Game>(new Game(glm::ivec2(800, 600))));
	return 0;
}