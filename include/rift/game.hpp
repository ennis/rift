#ifndef GAME_HPP
#define GAME_HPP

#include <common.hpp>
#include <engine.hpp>

class Renderer;

//==========================================
// classe Game
//
class Game
{
public:
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
};

#endif