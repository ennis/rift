#ifndef GL_COMMON_HPP
#define GL_COMMON_HPP

#include <renderer_common.hpp>
#include <gl_core_4_4.hpp>

struct ElementFormatInfoGL {
	// vertex element
	GLenum type;
	unsigned int size; 
	// pixel element
	GLenum internalFormat;
	GLenum externalFormat;
	// 
	bool normalize;
	bool compressed;
};

const ElementFormatInfoGL &getElementFormatInfoGL(ElementFormat format);

#endif /* end of include guard: GL_COMMON_HPP */