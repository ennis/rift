#ifndef UNIFORM_HPP
#define UNIFORM_HPP

#include <effect.hpp>

// Interface for working with shader paramters (uniforms)
// target: uniform location (var or buffer binding)
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
class ConstantValue : public Uniform
{
public:
	ConstantValue() = default;

	ConstantValue(Shader &shader, const char *name) :
		Uniform(shader, name),
		cb(sizeof(T), ResourceUsage::Dynamic, BufferUsage::ConstantBuffer, nullptr)
	{
		// assign a buffer binding point (none assigned yet)
		gl::UniformBlockBinding(shader.getProgram(), bufferLocation, bufferLocation);
	}

	ConstantValue(ConstantValue &&rhs)
	{
		*this = std::move(rhs);
	}

	ConstantValue &operator=(ConstantValue &&rhs)
	{
		cb = std::move(rhs.cb);
		Uniform::operator=(std::move(rhs));
		return *this;
	}

	void update(const T &data)
	{
		cb.update(0, sizeof(T), &data);
	}

	void bind(Renderer &renderer)
	{
		if (bufferLocation != -1)
			renderer.setConstantBuffer(bufferLocation, &cb);
	}

	Buffer *getBuffer()
	{
		return &cb;
	}

private:
	Buffer cb;
};

class ConstantBuffer : public Uniform
{
public:
	ConstantBuffer() = default;
	ConstantBuffer(ConstantBuffer &&rhs) { *this = std::move(rhs); }
	ConstantBuffer &operator=(ConstantBuffer &&rhs) {
		cb = std::move(rhs.cb);
		Uniform::operator=(std::move(rhs));
		return *this;
	}

	ConstantBuffer(Shader &shader, const char *name, Buffer *buf) : Uniform(shader, name), cb(buf)
	{
		// assign a buffer binding point (none assigned yet)
		gl::UniformBlockBinding(shader.getProgram(), bufferLocation, bufferLocation);
	}

	void bind(Renderer &renderer)
	{
		if (bufferLocation != -1)
			renderer.setConstantBuffer(bufferLocation, cb);
	}

private:
	Buffer *cb = nullptr;
};
 
#endif /* end of include guard: UNIFORM_HPP */