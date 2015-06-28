#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <rendering/opengl4.hpp>
#include <GLFW/glfw3.h>

struct ContextOptions
{
	ContextOptions() = default;
	// OpenGL 4.4 core profile
	int glMajor = 4;
	int glMinor = 4;
	int numSamples = 0;
	bool fullscreen = false;
};

class Application;

class MainLoop
{
public:
	virtual ~MainLoop() {}
	virtual void initialize(Application &app) = 0;
	virtual void render(float dt) = 0;
	virtual void update(float dt) = 0;
};

class Application
{
public:
	Application(const char *title, unsigned width, unsigned height, const ContextOptions &options);

	template <typename TMainLoop>
	void run()
	{
		mainLoop = std::make_unique<TMainLoop>();
		mainLoop->initialize(*this);
		runMainLoop();
	}

	void setTitle(const char *title);
	void getSize(unsigned &width, unsigned &height) const;
	GraphicsContext &getGraphicsContext();
	GLFWwindow *getWindow() const;
	void getLastScrollOffset(double &xoffset, double &yoffset);

	// internal
	void setLastScrollOffset(double xoffset, double yoffset);

private:
	void runMainLoop();
	double scrollOffsetX, scrollOffsetY;
	GLFWwindow *window;
	std::unique_ptr<GraphicsContext> graphicsContext;
	std::unique_ptr<MainLoop> mainLoop;
};

#endif