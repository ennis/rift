#include <engine.hpp>
#include <clock.hpp>
#include <entity.hpp>
#include <opengl.hpp>
#include <game.hpp>
#include <map>

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

	const std::map<GLenum, std::string> gl_debug_source_names = {
		{ GL_DEBUG_SOURCE_API, "DEBUG_SOURCE_API" },
		{ GL_DEBUG_SOURCE_APPLICATION, "DEBUG_SOURCE_APPLICATION" },
		{ GL_DEBUG_SOURCE_OTHER, "DEBUG_SOURCE_OTHER" },
		{ GL_DEBUG_SOURCE_SHADER_COMPILER, "DEBUG_SOURCE_SHADER_COMPILER" },
		{ GL_DEBUG_SOURCE_THIRD_PARTY, "DEBUG_SOURCE_THIRD_PARTY" },
		{ GL_DEBUG_SOURCE_WINDOW_SYSTEM, "DEBUG_SOURCE_WINDOW_SYSTEM" }
	};

	const std::map<GLenum, std::string> gl_debug_type_names = {
		{ GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "DEBUG_TYPE_DEPRECATED_BEHAVIOR" },
		{ GL_DEBUG_TYPE_ERROR, "DEBUG_TYPE_ERROR" },
		{ GL_DEBUG_TYPE_MARKER, "DEBUG_TYPE_MARKER" },
		{ GL_DEBUG_TYPE_OTHER, "DEBUG_TYPE_OTHER" },
		{ GL_DEBUG_TYPE_PERFORMANCE, "DEBUG_TYPE_PERFORMANCE" },
		{ GL_DEBUG_TYPE_POP_GROUP, "DEBUG_TYPE_POP_GROUP" },
		{ GL_DEBUG_TYPE_PORTABILITY, "DEBUG_TYPE_PORTABILITY" },
		{ GL_DEBUG_TYPE_PUSH_GROUP, "DEBUG_TYPE_PUSH_GROUP" },
		{ GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "DEBUG_TYPE_UNDEFINED_BEHAVIOR" }
	};

	const std::map<GLenum, std::string> gl_debug_severity_names = {
		{ GL_DEBUG_SEVERITY_HIGH, "DEBUG_SEVERITY_HIGH" },
		{ GL_DEBUG_SEVERITY_LOW, "DEBUG_SEVERITY_LOW" },
		{ GL_DEBUG_SEVERITY_MEDIUM, "DEBUG_SEVERITY_MEDIUM" },
		{ GL_DEBUG_SEVERITY_NOTIFICATION, "DEBUG_SEVERITY_NOTIFICATION" }
	};

	void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *data)
	{
		std::string src_str = "Unknown";
		if (gl_debug_source_names.count(source)) src_str = gl_debug_source_names.at(source);
		std::string type_str = "Unknown";
		if (gl_debug_type_names.count(type)) type_str = gl_debug_type_names.at(type);
		std::string sev_str = "Unknown";
		if (gl_debug_severity_names.count(severity)) sev_str = gl_debug_severity_names.at(severity);

		LOG << "(GL debug: " << id << ", " << src_str << ", " << type_str << ", " << sev_str << ") " << msg ;
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

	GLint v;
	glGetIntegerv(GL_CONTEXT_FLAGS, &v);
	if (v & GL_CONTEXT_FLAG_DEBUG_BIT) {
		LOG << "OpenGL debug context present";
		glDebugMessageCallback(debugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
		glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 1111, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
			"Started logging OpenGL messages");
	}


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

