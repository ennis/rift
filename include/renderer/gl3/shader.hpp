#ifndef SHADER_HPP
#define SHADER_HPP

#include <common.hpp>
#include <opengl.hpp>
#include <string>

GLuint compileShader(const char *shaderSource, GLenum type);
void linkProgram(GLuint program);


#endif