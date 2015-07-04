#include <application.hpp>
#include <log.hpp>
#include <gl_core_4_4.hpp>
#include <AntTweakBar.h>

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

	void GLFWErrorCallback(int error, const char* description)
	{
		ERROR << "GLFW error: " << description;
	}

	Application *gApplication = nullptr;
}

void Application::setLastScrollOffset(double xoffset, double yoffset)
{
	scrollOffsetX += xoffset;
	scrollOffsetY += yoffset;
}

Application::Application(const char *title, unsigned width, unsigned height, const ContextOptions &options) :
frameCount(0),
scrollOffsetX(0.0),
scrollOffsetY(0.0)
{
	gApplication = this;
	// GLFW
	glfwSetErrorCallback(GLFWErrorCallback);
	if (!glfwInit()) {
		ERROR << "Failed to initialize GLFW";
		assert(false);
	}
	LOG << "Using GLFW :\n" << glfwGetVersionString();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, options.glMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, options.glMinor);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, gl::TRUE_);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, gl::TRUE_);
	window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window) {
		ERROR << "Failed to create window";
		return;
	}
	glfwMakeContextCurrent(window);
	// TODO obviously, this should be in the backend
	if (!gl::sys::LoadFunctions()) {
		ERROR << "GL init failed";
		assert(false);
	}
	LOG << "OpenGL version: " << gl::GetString(gl::VERSION);
	LOG << "GLSL version: " << gl::GetString(gl::SHADING_LANGUAGE_VERSION);
	glfwSetCharCallback(window, GLFWCharHandler);
	glfwSetCursorEnterCallback(window, GLFWCursorEnterHandler);
	glfwSetCursorPosCallback(window, GLFWCursorPosHandler);
	glfwSetKeyCallback(window, GLFWKeyHandler);
	glfwSetMouseButtonCallback(window, GLFWMouseButtonHandler);
	glfwSetScrollCallback(window, GLFWScrollHandler);
	graphicsContext = std::make_unique<GraphicsContext>();
}

GraphicsContext &Application::getGraphicsContext()
{
	return *graphicsContext;
}

void Application::setTitle(const char *title)
{
	glfwSetWindowTitle(window, title);
}

GLFWwindow *Application::getWindow() const
{
	return window;
}

void Application::getLastScrollOffset(double &xoffset, double &yoffset)
{
	xoffset = scrollOffsetX;
	yoffset = scrollOffsetY;
}

void Application::runMainLoop()
{
	using namespace std::chrono;
	try {
		qpc_clock::time_point tb = qpc_clock::now();
		qpc_clock::time_point tf;
		// main loop
		while (!glfwWindowShouldClose(window)) {
			tf = qpc_clock::now();
			duration<float> frame = duration_cast<nanoseconds>(tf - tb);
			tb = tf;
			lastDeltaTime = frame.count();
			graphicsContext->beginFrame();
			mainLoop->render(lastDeltaTime);
			graphicsContext->endFrame();
			mainLoop->update(lastDeltaTime);
			glfwSwapBuffers(window);
			glfwPollEvents();
			++frameCount;
		}
	}
	catch (std::exception &e) {
		// catch-all
		ERROR << "std::exception: " << e.what();
	}

	mainLoop.reset();
	graphicsContext.reset();
	glfwDestroyWindow(window);
}


Application *GetApplication()
{
	return gApplication;
}


