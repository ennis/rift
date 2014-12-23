#include <engine.hpp>
#include <clock.hpp>
#include <entity.hpp>
#include <opengl.hpp>
#include <game.hpp>

Engine *Engine::gEngine = nullptr;

namespace
{
	// GLFW event handlers
	void GLFWMouseButtonHandler(GLFWwindow *window, int button, int action, int mods)
	{
		if (!TwEventMouseButtonGLFW(button, action)) {
			// not handled, do something
		}
	}

	void GLFWCursorPosHandler(GLFWwindow *window, double xpos, double ypos)
	{
		if (!TwEventMousePosGLFW((int)xpos, (int)ypos)) {
			// ...
		}
	}

	void GLFWCursorEnterHandler(GLFWwindow *window, int entered)
	{
		// ...
	}

	void GLFWScrollHandler(GLFWwindow *window, double xoffset, double yoffset)
	{
		if (!TwEventMouseWheelGLFW((int)xoffset)) {
			// ...
		}
	}

	void GLFWKeyHandler(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (!TwEventKeyGLFW(key, action)) {
			// ...
		}
	}

	void GLFWCharHandler(GLFWwindow *window, unsigned int codepoint)
	{
		if (!TwEventCharGLFW(codepoint, GLFW_PRESS)) {
			// ...
		}
	}

}

Engine::Engine(Window &window) : 
mWindow(window)
{
	gEngine = this;
	init();
}

void Engine::init()
{
	// init GLEW
	glewExperimental = true;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		ERROR << "glewInit failed: " << glewGetErrorString(err);
		return;
	}
	LOG << "Using GLEW " << glewGetString(GLEW_VERSION);
	LOG << "OpenGL version: " << glGetString(GL_VERSION);
	LOG << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
	
	// init anttweakbar
	TwInit(TW_OPENGL_CORE, NULL);
	auto winSize = mWindow.size();
	TwWindowSize(winSize.x, winSize.y);
	
	// set glfw callbacks
	auto winHandle = mWindow.getHandle();
	glfwSetCharCallback(winHandle, GLFWCharHandler);
	glfwSetCursorEnterCallback(winHandle, GLFWCursorEnterHandler);
	glfwSetCursorPosCallback(winHandle, GLFWCursorPosHandler);
	glfwSetKeyCallback(winHandle, GLFWKeyHandler);
	glfwSetMouseButtonCallback(winHandle, GLFWMouseButtonHandler);
	glfwSetScrollCallback(winHandle, GLFWScrollHandler);
	
	// initialize the renderer
	mRenderer.initialize();
}

void Engine::run(Game &game)
{
	mGame = &game;
	mainLoop();
}

void Engine::mainLoop()
{
	using namespace std::chrono;
	try {
		qpc_clock::time_point tb = qpc_clock::now();
		qpc_clock::time_point tf;
		// main loop
		while (!mWindow.shouldClose())
		{
			tf = qpc_clock::now();
			duration<float> frame = duration_cast<nanoseconds>(tf - tb);
			tb = tf;
			float dt = frame.count();
			mGame->render(dt);
			// update all entities
			for (auto &&ent : Entity::getList()) {
				ent.update(dt);
			}
			mGame->update(dt);
			mWindow.swapBuffers();
			mWindow.pollEvents();
		}
		mWindow.close();
	}
	catch (std::exception &e) {
		// catch-all
		ERROR << "std::exception: " << e.what();
	}
}

