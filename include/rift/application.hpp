#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <rendering/opengl4.hpp>
#include <GLFW/glfw3.h>
#include <clock.hpp>

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

	int getWidth() const
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		return width;
	}

	int getHeight() const
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		return height;
	}

	GraphicsContext &getGraphicsContext();
	void getLastScrollOffset(double &xoffset, double &yoffset);

	// internal
	void setLastScrollOffset(double xoffset, double yoffset);
	GLFWwindow *getWindow() const;

	double getTime() const
	{
		using namespace std::chrono;
		auto now = qpc_clock::now();
		return duration_cast<duration<double>>(now - startTime).count();
	}

	double getDeltaTime() const
	{
		return lastDeltaTime;
	}

	unsigned long getFrameCount() const 
	{
		return frameCount;
	}

private:
	qpc_clock::time_point startTime;
	double lastDeltaTime;
	unsigned long frameCount;
	void runMainLoop();
	double scrollOffsetX, scrollOffsetY;
	GLFWwindow *window;
	std::unique_ptr<GraphicsContext> graphicsContext;
	std::unique_ptr<MainLoop> mainLoop;
};

Application *GetApplication();

#endif