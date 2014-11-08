#include <shader.hpp>
#include <log.hpp>
#include <gl3error.hpp>

GLuint compileShader(const char *shaderSource, GLenum type)
{
	GLuint obj = glCreateShader(type);
	const char *shaderSources[1] = { shaderSource };
	GLCHECK(glShaderSource(obj, 1, shaderSources, NULL));
	GLCHECK(glCompileShader(obj));

	GLint status = GL_TRUE;
	GLint logsize = 0;

	glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &logsize);
	if (status != GL_TRUE) {
		ERROR << "Compile error:";
		if (logsize != 0) {
			char *logbuf = new char[logsize];
			glGetShaderInfoLog(obj, logsize, &logsize, logbuf);
			ERROR << logbuf;
			delete[] logbuf;
			GLCHECK(glDeleteShader(obj));
		}
		else {
			ERROR << "<no log>";
		}
		throw std::runtime_error("shader compilation failed");
	}

	return obj;
}

void linkProgram(GLuint program)
{
	GLint status = GL_TRUE;
	GLint logsize = 0;

	GLCHECK(glLinkProgram(program));
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logsize);
	if (status != GL_TRUE) {
		ERROR << "Link error:";
		if (logsize != 0) {
			char *logbuf = new char[logsize];
			glGetProgramInfoLog(program, logsize, &logsize, logbuf);
			ERROR << logbuf;
			delete[] logbuf;
		}
		else {
			ERROR << "<no log>";
		}
		throw std::runtime_error("link failed");
	}
}
