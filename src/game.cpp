#include <game.hpp>
#include <log.hpp>
#include <clock.hpp>
#include <stdexcept>
#include <string>
#include <renderer.hpp>
#include <entity.hpp>
#include <sharedresources.hpp>

// TODO
#include <AntTweakBar.h>

GLFWwindow *Game::sWindow = nullptr;
glm::ivec2 Game::sWindowSize;
std::unique_ptr<Game> Game::sGameInstance = nullptr;
std::unique_ptr<Renderer> Game::sRenderer = nullptr; 
bool Game::sWindowResized;

TwBar *sMainBar = nullptr;

static void GLFWMouseButtonHandler(GLFWwindow *window, int button, int action, int mods);
static void GLFWCursorPosHandler(GLFWwindow *window, double xpos, double ypos);
static void GLFWCursorEnterHandler(GLFWwindow *window, int entered);
static void GLFWScrollHandler(GLFWwindow *window, double xoffset, double yoffset);
static void GLFWKeyHandler(GLFWwindow *window, int key, int scancode, int action, int mods);
static void GLFWCharHandler(GLFWwindow *window, unsigned int codepoint);

Game::Game(glm::ivec2 const &windowSize)
{
	sWindowSize = windowSize;
	sWindow = nullptr;
}

Game::~Game()
{}

static void errorCallback(int error, const char* description)
{
	ERROR << "GLFW error: " << description;
}

int Game::initContext()
{
	LOG << "setting up render window";
	//====== GLFW ======
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) {
		ERROR << "Failed to initialize GLFW";
		return EXIT_FAILURE;
	}
	LOG << "Using GLFW :\n" << glfwGetVersionString();
	// OpenGL 4.3 core profile
	// Supported by GeForce 400 and later, Radeon HD 5000 and later (around 2010)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_SAMPLES, 8);
	sWindow = glfwCreateWindow(sWindowSize.x, sWindowSize.y, "Rift", NULL, NULL);
	if (!sWindow) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(sWindow);
	//====== GLEW ======
	glewExperimental = true;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		ERROR << "glewInit failed: " << glewGetErrorString(err);
		return EXIT_FAILURE;
	}
	LOG << "Using GLEW " << glewGetString(GLEW_VERSION);
	LOG << "OpenGL version: " << glGetString(GL_VERSION);
	LOG << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
	// init anttweakbar
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(sWindowSize.x, sWindowSize.y);
	// set glfw callbacks
	glfwSetCharCallback(sWindow, GLFWCharHandler);
	glfwSetCursorEnterCallback(sWindow, GLFWCursorEnterHandler);
	glfwSetCursorPosCallback(sWindow, GLFWCursorPosHandler);
	glfwSetKeyCallback(sWindow, GLFWKeyHandler);
	glfwSetMouseButtonCallback(sWindow, GLFWMouseButtonHandler);
	glfwSetScrollCallback(sWindow, GLFWScrollHandler);
	// initialize the renderer
	sRenderer = std::unique_ptr<Renderer>(new Renderer());
	sRenderer->initialize();
	// yep
	// bullshit sense tingling
	initSharedResources(*sRenderer);
	return 0;
}

// GLFW event handlers
static void GLFWMouseButtonHandler(GLFWwindow *window, int button, int action, int mods)
{
	if (!TwEventMouseButtonGLFW(button, action)) {
		// not handled, do something
	}
}

static void GLFWCursorPosHandler(GLFWwindow *window, double xpos, double ypos)
{
	if (!TwEventMousePosGLFW((int)xpos, (int)ypos)) {
		// ...
	}
}

static void GLFWCursorEnterHandler(GLFWwindow *window, int entered)
{
	// ...
}

static void GLFWScrollHandler(GLFWwindow *window, double xoffset, double yoffset)
{
	if (!TwEventMouseWheelGLFW((int)xoffset)) {
		// ...
	}
}

static void GLFWKeyHandler(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (!TwEventKeyGLFW(key, action)) {
		// ...
	}
}

static void GLFWCharHandler(GLFWwindow *window, unsigned int codepoint)
{
	if (!TwEventCharGLFW(codepoint, GLFW_PRESS)) {
		// ...
	}
}

int Game::run()
{
	using namespace std::chrono;
	try {
		if (initContext()) {
			ERROR << "Application::run : Context initialization failed";
			return EXIT_FAILURE;
		}
		// user init
		init();
		qpc_clock::time_point tb = qpc_clock::now();
		qpc_clock::time_point tf;
		// main loop
		while (!glfwWindowShouldClose(sWindow))
		{
			int width, height;
			sWindowResized = false;
			glfwGetWindowSize(sWindow, &width, &height);
			if (width != sWindowSize.x || height != sWindowSize.y) {	
				sWindowSize.x = width;
				sWindowSize.y = height;
				sWindowResized = true;
				LOG << "Window resized";
			}
			tf = qpc_clock::now();
			duration<float> frame = duration_cast<nanoseconds>(tf - tb);
			tb = tf;
			float dt = frame.count();
			render(dt);
			// update all entities
			for (auto &&ent : Entity::getList()) {
				ent.update(dt);
			}
			update(dt);
			glfwSwapBuffers(sWindow);
			glfwPollEvents();
		}
		glfwDestroyWindow(sWindow);
		glfwTerminate();
	}
	catch (std::exception &e) {
		// catch-all
		ERROR << "std::exception: " << e.what();
	}
	return EXIT_SUCCESS;
}

void Game::run(std::unique_ptr<Game> instance)
{
	Game::sGameInstance = std::move(instance);
	sGameInstance->run();
}
