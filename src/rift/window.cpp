#include <window.hpp>
#include <log.hpp>

namespace
{
	void errorCallback(int error, const char* description)
	{
		ERROR << "GLFW error: " << description;
	}

}

Window::Window(
	const char *title,
	unsigned int width, 
	unsigned int height, 
	ContextOptions const &options) : 
	mOptions(options)
{
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) {
		ERROR << "Failed to initialize GLFW";
		return;
	}
	LOG << "Using GLFW :\n" << glfwGetVersionString();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, options.glMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, options.glMinor);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	mHandle = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!mHandle) {
		ERROR << "Failed to create window";
		return;
	}
	glfwMakeContextCurrent(mHandle);

}

bool Window::shouldClose()
{
	return glfwWindowShouldClose(mHandle);
}

void Window::pollEvents()
{
	return glfwPollEvents();
}

void Window::close()
{
	glfwDestroyWindow(mHandle);
}

void Window::swapBuffers()
{
	glfwSwapBuffers(mHandle);
}


void Window::setTitle(const char *title)
{
	glfwSetWindowTitle(mHandle, title);
}

Window::~Window()
{
	glfwTerminate();
}
