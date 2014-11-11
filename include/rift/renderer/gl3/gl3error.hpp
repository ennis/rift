#ifndef GL3ERROR_HPP
#define GL3ERROR_HPP

#include <opengl.hpp>

// http://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/
void check_gl_error(const char *file, int line);

///
/// Usage
/// [... some opengl calls]
/// glCheckError();
///
#define GLCHECK(expr) do {expr; check_gl_error(__FILE__, __LINE__); } while(0)

#endif