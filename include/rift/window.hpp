#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <opengl.hpp>

struct ContextOptions
{
	ContextOptions() = default;
	
	// OpenGL 4.3 core profile
	// Supported by GeForce 400 and later, Radeon HD 5000 and later (around 2010)
	int glMajor = 4;
	int glMinor = 3;
	int numSamples = 0;
	bool fullscreen = false;
};

class Window
{
public:
	Window(
		const char *title, 
		unsigned int width, 
		unsigned int height, 
		ContextOptions const &options = ContextOptions());
	Window(Window const &window) = delete;
	~Window();

	bool shouldClose();
	void pollEvents();
	void close();
	void swapBuffers();
	void setTitle(const char *title);

	glm::ivec2 size() {
		int width, height;
		glfwGetWindowSize(mHandle, &width, &height);
		return glm::ivec2(width, height);
	}

	GLFWwindow *getHandle() {
		return mHandle;
	}

private:
	ContextOptions mOptions;
	GLFWwindow *mHandle;
};

 
#endif /* end of include guard: WINDOW_HPP */