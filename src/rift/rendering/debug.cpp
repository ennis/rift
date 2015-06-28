#include <rendering/opengl4.hpp>
#include <map>
#include <log.hpp>

const std::map<GLenum, std::string> gl_debug_source_names = {
	{ gl::DEBUG_SOURCE_API, "DEBUG_SOURCE_API" },
	{ gl::DEBUG_SOURCE_APPLICATION, "DEBUG_SOURCE_APPLICATION" },
	{ gl::DEBUG_SOURCE_OTHER, "DEBUG_SOURCE_OTHER" },
	{ gl::DEBUG_SOURCE_SHADER_COMPILER, "DEBUG_SOURCE_SHADER_COMPILER" },
	{ gl::DEBUG_SOURCE_THIRD_PARTY, "DEBUG_SOURCE_THIRD_PARTY" },
	{ gl::DEBUG_SOURCE_WINDOW_SYSTEM, "DEBUG_SOURCE_WINDOW_SYSTEM" }
};

const std::map<GLenum, std::string> gl_debug_type_names = {
	{ gl::DEBUG_TYPE_DEPRECATED_BEHAVIOR, "DEBUG_TYPE_DEPRECATED_BEHAVIOR" },
	{ gl::DEBUG_TYPE_ERROR, "DEBUG_TYPE_ERROR" },
	{ gl::DEBUG_TYPE_MARKER, "DEBUG_TYPE_MARKER" },
	{ gl::DEBUG_TYPE_OTHER, "DEBUG_TYPE_OTHER" },
	{ gl::DEBUG_TYPE_PERFORMANCE, "DEBUG_TYPE_PERFORMANCE" },
	{ gl::DEBUG_TYPE_POP_GROUP, "DEBUG_TYPE_POP_GROUP" },
	{ gl::DEBUG_TYPE_PORTABILITY, "DEBUG_TYPE_PORTABILITY" },
	{ gl::DEBUG_TYPE_PUSH_GROUP, "DEBUG_TYPE_PUSH_GROUP" },
	{ gl::DEBUG_TYPE_UNDEFINED_BEHAVIOR, "DEBUG_TYPE_UNDEFINED_BEHAVIOR" }
};

const std::map<GLenum, std::string> gl_debug_severity_names = {
	{ gl::DEBUG_SEVERITY_HIGH, "DEBUG_SEVERITY_HIGH" },
	{ gl::DEBUG_SEVERITY_LOW, "DEBUG_SEVERITY_LOW" },
	{ gl::DEBUG_SEVERITY_MEDIUM, "DEBUG_SEVERITY_MEDIUM" },
	{ gl::DEBUG_SEVERITY_NOTIFICATION, "DEBUG_SEVERITY_NOTIFICATION" }
};

void APIENTRY debugCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *msg,
	const void *data)
{
	std::string src_str = "Unknown";
	if (gl_debug_source_names.count(source)) src_str = gl_debug_source_names.at(source);
	std::string type_str = "Unknown";
	if (gl_debug_type_names.count(type)) type_str = gl_debug_type_names.at(type);
	std::string sev_str = "Unknown";
	if (gl_debug_severity_names.count(severity)) sev_str = gl_debug_severity_names.at(severity);

	//if (severity != gl::DEBUG_SEVERITY_LOW && severity != gl::DEBUG_SEVERITY_NOTIFICATION)
	LOG << "(GL debug: " << id << ", " << src_str << ", " << type_str << ", " << sev_str << ") " << msg;
}

void setDebugCallback()
{
	gl::Enable(gl::DEBUG_OUTPUT_SYNCHRONOUS);
	gl::DebugMessageCallback(debugCallback, nullptr);
	gl::DebugMessageControl(gl::DONT_CARE, gl::DONT_CARE, gl::DONT_CARE, 0, nullptr, true);
	gl::DebugMessageInsert(
		gl::DEBUG_SOURCE_APPLICATION,
		gl::DEBUG_TYPE_MARKER,
		1111,
		gl::DEBUG_SEVERITY_NOTIFICATION, -1,
		"Started logging OpenGL messages");
}