#ifndef SHADER_HPP
#define SHADER_HPP

#include <globject.hpp>
#include <string>
#include <utility>	// move

class Shader 
{
public:
	friend class Renderer;
	Shader() = default;
	Shader(const Shader&) = delete;
	Shader &operator=(const Shader&) = delete;
	Shader(Shader &&rhs)
	{
		*this = std::move(rhs);
	}
	Shader &operator=(Shader&& rhs)
	{
		id = rhs.id;
		rhs.id = 0;
		return *this;
	}

	~Shader()
	{
		gl::DeleteProgram(id);
	}

	Shader(std::string vsSource_, std::string psSource_);

	bool isNull() const
	{
		return id == 0;
	}

private:
	void swap(Shader &&rhs);

	GLuint id; 
	std::string vsSource;
	std::string psSource;
};
 
#endif /* end of include guard: SHADER_HPP */