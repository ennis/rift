#ifndef SHADERCOMPILER_HPP
#define SHADERCOMPILER_HPP

#include <rendering/opengl4/opengl4.hpp>
#include <array_ref.hpp>
#include <string>

namespace gl4
{
	struct Keyword
	{
		std::string define;
		std::string value;
	};

	ShaderProgram::Ptr compileShader(
		const char *source,
		const char *include_path,
		ShaderStage stage,
		util::array_ref<Keyword> keywords);

	std::string loadShaderSource(const char *path);

	// internal
	//GLuint compileShader(const char *shaderSource, GLenum type);
}

 
#endif /* end of include guard: EFFECT_HPP */