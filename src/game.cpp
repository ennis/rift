#include <game.hpp>
#include <log.hpp>
#include <clock.hpp>
#include <stdexcept>
#include <string>

#include <renderer/gl3/gl3renderer.hpp>

GLFWwindow *Game::sWindow = nullptr;
glm::ivec2 Game::sWindowSize;
std::unique_ptr<Game> Game::sGameInstance = nullptr;
std::unique_ptr<Renderer> Game::sRenderer = nullptr;

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
	LOG << "--- setting up render window ----------------";
	//====== GLFW ======
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) {
		ERROR << "Failed to initialize GLFW";
		return EXIT_FAILURE;
	}
	LOG << "Using GLFW :\n" << glfwGetVersionString();

	// OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
	sRenderer = std::unique_ptr<GL3Renderer>(new GL3Renderer);
	sRenderer->initialize();

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

//============================================================================
//
//
//

void Game::init()
{
	// rien à faire
}


void Game::render(float dt)
{
	renderer().render();
}

void Game::update(float dt)
{
	mLastTime += dt;
	mTotalTime += dt;

	// mise à jour du compteur FPS toutes les secondes
	if (mLastTime > 1.f) {
		mLastTime = 0.f;
		glfwSetWindowTitle(Game::window(), ("Rift (" + std::to_string(1.f / dt) + " FPS)").c_str());
	}
}
