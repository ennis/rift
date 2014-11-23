#ifndef OPENGL_HPP
#define OPENGL_HPP

// all things opengl

#ifdef _MSC_VER
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <GLFW/glfw3.h>

#endif
