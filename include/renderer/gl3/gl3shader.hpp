#ifndef GL3SHADER_HPP
#define GL3SHADER_HPP

#include <common.hpp>
#include <vector>
#include <string>
#include <opengl.hpp>

//
// Chargement d'un shader depuis un fichier
std::string loadShaderSource(const char *path);

class GLProgram
{
public:
	GLProgram() = default;

	// chargement rapide depuis un fichier
	void loadFromFile(const char *vsPath, const char *fsPath);

	void addShaderSource(const char *src, GLenum type);
	void link();
	void cleanup();

	void use();

	void uniform1f(const char *name, float value);
	void uniform2f(const char *name, glm::vec2 const &value);
	void uniform3f(const char *name, glm::vec3 const &value);
	void uniform4f(const char *name, glm::vec4 const &value);
	void uniform1i(const char *name, int value);
	void uniform2i(const char *name, glm::ivec2 const &value);
	void uniformMatrix4fv(const char *name, glm::mat4 const &value);

	std::vector<GLuint> shaders;
	GLuint program = -1;
};

#endif