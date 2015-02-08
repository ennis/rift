#ifndef UNIFORM_HPP
#define UNIFORM_HPP

#include <effect.hpp>


// Interface for working with shader paramters (uniforms)
// target: uniform location (var or buffer binding)

// base class: contains common code for binding and type checking?
class Uniform
{
public:
	Uniform() = default;

	Uniform(Shader &shader, const char *name)
	{
		bufferLocation = shader.getBufferLocation(name);
	}

	Uniform(const Uniform &) = delete;
	Uniform &operator=(const Uniform &) = delete;

	Uniform(Uniform &&rhs)
	{
		*this = std::move(rhs);
	}

	Uniform &operator=(Uniform &&rhs)
	{
		bufferLocation = rhs.bufferLocation;
		return *this;
	}

protected:
	unsigned int bufferLocation = 0;
};

// target: uniform var or buffer; contains: copy of value
template <typename T>
class ConstantBuffer : public Uniform
{
public:
	ConstantBuffer() = default;

	ConstantBuffer(Shader &shader, const char *name) :
		Uniform(shader, name),
		cb(sizeof(T), ResourceUsage::Dynamic, BufferUsage::ConstantBuffer, nullptr)
	{
	}

	ConstantBuffer(ConstantBuffer &&rhs)
	{
		*this = std::move(rhs);
	}

	ConstantBuffer &operator=(ConstantBuffer &&rhs)
	{
		value = std::move(rhs.value);
		cb = std::move(rhs.cb);
		Uniform::operator=(std::move(rhs));
		return *this;
	}

	// returns a mutable reference to the value
	T& get() 
	{
		return value;
	}

	void update()
	{
		cb.update(0, sizeof(T), &value);
	}

	void bind(Renderer &renderer)
	{
		if (bufferLocation != -1)
			renderer.setConstantBuffer(bufferLocation, &cb);
	}

private:
	T value;
	Buffer cb;
};

 
#endif /* end of include guard: UNIFORM_HPP */