#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <globject.hpp>

class Buffer : public GLAPIObject
{
public:
	GL_MOVEABLE_OBJECT_IMPL(Buffer)
	GL_IS_NULL_IMPL(id)

	void swap(Buffer &&rhs) 
	{
		std::swap(id, rhs.id);
	}

	~Buffer()
	{
		if (id) {
			gl::DeleteBuffers(1, &id);
		}
	}

	void update(std::size_t offset, std::size_t size, const void *data)
	{
		if (gl::exts::var_EXT_direct_state_access) {
			gl::NamedBufferSubDataEXT(id, offset, size, data);
		}
		else {
			gl::BindBuffer(bindingPoint, id);
			gl::BufferSubData(bindingPoint, offset, size, data);
		}
	}

private:
	Buffer(GLenum bindingPoint_, std::size_t size_, const void *data, ResourceUsage usage_, GLbitfield flags) :
	usage(usage_), bindingPoint(bindingPoint_), size(size_)
	{
		gl::GenBuffers(1, &id);
		gl::BindBuffer(bindingPoint, id);
		// allocate immutable storage
		gl::BufferStorage(bindingPoint, size, data, flags);
		gl::BindBuffer(bindingPoint, 0);
	}

	GLuint id;
	ResourceUsage usage;
	GLenum bindingPoint;
	std::size_t size;
};

 
#endif /* end of include guard: BUFFER_HPP */