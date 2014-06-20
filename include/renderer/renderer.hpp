#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <common.hpp>

class Renderer
{
public:
	//
	// initialization
	virtual void initialize() = 0;


	//
	// render
	virtual void render() = 0;

	//
	// 
	virtual void setClearColor(glm::vec4 const &color) = 0;
	virtual void setClearDepth(float color) = 0;

};

#endif