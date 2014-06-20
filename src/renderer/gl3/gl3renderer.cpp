#include <renderer/gl3/gl3renderer.hpp>
#include <log.hpp>
#include <game.hpp>

void GL3Renderer::initialize()
{
	LOG << "Initialized GL3 renderer";
}

void GL3Renderer::render()
{
	// état par défaut du pipeline
	glBindFramebuffer(0, GL_FRAMEBUFFER);
	// 226.f / 255.f, 236.f / 255.f, 124.f / 255.f, 1.f
	glClearColor(mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a);
	glClearDepth(mClearDepth);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void GL3Renderer::setClearColor(glm::vec4 const &color)
{
	mClearColor = color;
}

void GL3Renderer::setClearDepth(float depth)
{
	mClearDepth = depth;
}