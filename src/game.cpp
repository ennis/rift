#include <game.hpp>
#include <log.hpp>
#include <clock.hpp>
#include <stdexcept>
#include <string>

#include <gl3rendererimpl.hpp>
#include <textureloader.hpp>

GLFWwindow *Game::sWindow = nullptr;
glm::ivec2 Game::sWindowSize;
std::unique_ptr<Game> Game::sGameInstance = nullptr;
std::unique_ptr<CRenderer> Game::sRenderer = nullptr;

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

	// OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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

	// initialize the renderer
	sRenderer = std::unique_ptr<CRenderer>(new CRenderer);
	sRenderer->initialize(std::unique_ptr<CGL3RendererImpl>(new CGL3RendererImpl()));

	return 0;
}

int Game::run()
{
	try {
		if (initContext()) {
			ERROR << "Application::run : Context initialization failed";
			return EXIT_FAILURE;
		}

		using namespace std::chrono;

		// user init
		init();
		qpc_clock::time_point tb = qpc_clock::now();
		qpc_clock::time_point tf;

		// main loop
		while (!glfwWindowShouldClose(sWindow))
		{
			int width, height;
			glfwGetWindowSize(sWindow, &width, &height);
			tf = qpc_clock::now();
			duration<float> frame = duration_cast<nanoseconds>(tf - tb);
			tb = tf;
			render(frame.count());
			update(frame.count());

			glfwSwapBuffers(sWindow);
			glfwPollEvents();
		}

		// end of loop
		// destroy all resources
		ResourceManager::getInstance().unloadAll();

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
