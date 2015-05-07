#ifndef GL_COMMON_HPP
#define GL_COMMON_HPP

#include <renderer_common.hpp>
#include <gl_core_4_4.hpp>

namespace gl4
{
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
	GLint textureFilterToGL(TextureFilter filter);
	GLint textureAddressModeToGL(TextureAddressMode addr);
	GLenum bufferUsageToBindingPoint(BufferUsage bufferUsage);
	GLenum blendOpToGL(BlendOp bo);
	GLenum blendFactorToGL(BlendFactor bf);
	GLenum cullModeToGLenum(CullMode mode);
	GLenum fillModeToGLenum(PolygonFillMode fillMode);
	GLenum primitiveTypeToGLenum(PrimitiveType type);
	GLenum stencilOpToGLenum(StencilOp op);
	GLenum stencilFuncToGLenum(StencilFunc func);
	GLenum shaderStageToGLenum(ShaderStage stage);
}

#endif /* end of include guard: GL_COMMON_HPP */