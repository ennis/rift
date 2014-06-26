#include <gl3shader.hpp>
#include <fstream>
#include <stdexcept>
#include <log.hpp>
#include <gl3error.hpp>

static std::string loadShaderSource(const char *path)
{
	char *src = NULL;
	std::ifstream shader;
	shader.open(path, std::ios_base::in | std::ios_base::binary);
	if (shader.fail()) {
		ERROR << "Could not open shader file " << path;
		throw std::runtime_error("Could not open shader file");
	}
	// get file size
	shader.seekg(0, std::ios_base::end);
	std::size_t size = shader.tellg();
	shader.seekg(0, std::ios_base::beg);
	LOG << path << ": size = " << size;

	std::string str;
	str.reserve(size);

	str.assign(
		std::istreambuf_iterator<char>(shader),
		std::istreambuf_iterator<char>());

	return str;
}

static GLuint loadShader(char const *shaderSource, GLenum type)
{
	GLuint obj = glCreateShader(type);
	GLCHECK(glShaderSource(obj, 1, &shaderSource, NULL));
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

static GLuint loadShaderFromFile(char const *shaderFileName, GLenum type)
{
	std::string src = loadShaderSource(shaderFileName);
	GLuint obj = loadShader(src.c_str() , type);
	return obj;
}

static int linkProgram(GLuint program)
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

	return 0;
}

void GLProgram::addShaderSource(const char *src, GLenum type)
{
	if (program == -1) {
		program = glCreateProgram();
	}

	GLuint shader = loadShader(src, type);
	shaders.push_back(shader);
	GLCHECK(glAttachShader(program, shader));
}

void GLProgram::cleanup()
{
	for (auto s : shaders) {
		GLCHECK(glDeleteShader(s));
	}
	GLCHECK(glDeleteProgram(program));
}

void GLProgram::link()
{
	if (linkProgram(program) == -1) {
		cleanup();
		assert(false);
	}
}

void GLProgram::use()
{
	assert(program != -1);
	GLCHECK(glUseProgram(program));
}

void GLProgram::loadFromFile(const char *vsPath, const char *fsPath)
{
	std::string vsSource = loadShaderSource(vsPath);
	std::string fsSource = loadShaderSource(fsPath);
	addShaderSource(vsSource.c_str(), GL_VERTEX_SHADER);
	addShaderSource(fsSource.c_str(), GL_FRAGMENT_SHADER);
	link();
}


void GLProgram::uniform1f(const char *name, float value) 
{
	assert(program != -1);
	GLuint location = glGetUniformLocation(program, name);
	glUniform1f(location, value);
}

void GLProgram::uniform2f(const char *name, glm::vec2 const &value) 
{
	assert(program != -1);
	GLuint location = glGetUniformLocation(program, name);
	glUniform2fv(location, 1, glm::value_ptr(value));

}

void GLProgram::uniform3f(const char *name, glm::vec3 const &value) 
{
	assert(program != -1);
	GLuint location = glGetUniformLocation(program, name);
	glUniform3fv(location, 1, glm::value_ptr(value));
}

void GLProgram::uniform4f(const char *name, glm::vec4 const &value) 
{
	assert(program != -1);
	GLuint location = glGetUniformLocation(program, name);
	glUniform4fv(location, 1, glm::value_ptr(value));
}

void GLProgram::uniform1i(const char *name, int value) 
{
	assert(program != -1);
	GLuint location = glGetUniformLocation(program, name);
	glUniform1i(location, value);
}

void GLProgram::uniform2i(const char *name, glm::ivec2 const &value) 
{
	assert(program != -1);
	GLuint location = glGetUniformLocation(program, name);
	glUniform2iv(location, 1, glm::value_ptr(value));
}

void GLProgram::uniformMatrix4fv(const char *name, glm::mat4 const &value)
{
	assert(program != -1);
	GLuint location = glGetUniformLocation(program, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

GLuint GLProgram::getUniformBlockIndex(const char *name)
{
	assert(program != -1);
	GLuint location = glGetUniformBlockIndex(program, name);
	return location;
}
