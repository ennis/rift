#ifndef SHADER_HPP
#define SHADER_HPP

#include <globject.hpp>
#include <string>

class Shader : public GLAPIObject
{
public:
	GL_MOVEABLE_OBJECT_IMPL(Shader)
	GL_IS_NULL_IMPL(id)

	~Shader()
	{//TODO
	}

	Shader(std::string vsSource_, std::string psSource_);

private:
	void swap(Shader &&rhs);

	GLuint id; 
	std::string vsSource;
	std::string psSource;
};
 
#endif /* end of include guard: SHADER_HPP */