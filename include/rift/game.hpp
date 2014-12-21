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
	// Méthode appelée lors de l'initialisation 
	virtual void init() = 0;

	//
	// Méthode appelée au moment du rendu
	virtual void render(float dt) = 0;

	//
	// Méthode appelée après le rendu
	// fusionner avec render?
	virtual void update(float dt) = 0;
};

#endif