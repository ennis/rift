#ifndef GL3RENDERER_HPP
#define GL3RENDERER_HPP

#include <renderer/renderer.hpp>

class GL3Renderer : public Renderer
{
public:
	GL3Renderer() = default;

	//
	// initialization
	void initialize();

	//
	// render
	void render();

	//
	// 
	void setClearColor(glm::vec4 const &color);
	void setClearDepth(float depth);

private:
	glm::vec4 mClearColor = glm::vec4(226.f / 255.f, 236.f / 255.f, 124.f / 255.f, 1.f);
	float mClearDepth = 100.f;
};

#endif