#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <globject.hpp>

namespace
{
	GLenum bufferUsageToBindingPoint(BufferUsage bufferUsage)
	{
		switch (bufferUsage)
		{
		case BufferUsage::VertexBuffer:
			return gl::ARRAY_BUFFER;
		case BufferUsage::IndexBuffer:
			return gl::ELEMENT_ARRAY_BUFFER;
		case BufferUsage::ConstantBuffer:
			return gl::UNIFORM_BUFFER;
		case BufferUsage::Unspecified:
		default:
			return gl::ARRAY_BUFFER;
		}
	}
}

class Buffer : public GLAPIObject
{
public:
	GL_MOVEABLE_OBJECT_IMPL(Buffer)
	GL_IS_NULL_IMPL(id)

	void swap(Buffer &&rhs) 
	{
		std::swap(id, rhs.id);
		std::swap(usage, rhs.usage);
		std::swap(bindingPoint, rhs.bindingPoint);
		std::swap(size, rhs.size);
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


	Buffer(std::size_t size,
		ResourceUsage resourceUsage,
		BufferUsage bufferUsage,
		const void* initialData) : Buffer(bufferUsageToBindingPoint(bufferUsage), size, initialData, resourceUsage, 0)
	{
	}

private:
	Buffer(GLenum bindingPoint_, std::size_t size_, const void *data, ResourceUsage usage_, GLbitfield flags) :
		usage(usage_), bindingPoint(bindingPoint_), size(size_)
	{
		gl::GenBuffers(1, &id);
		gl::BindBuffer(bindingPoint, id);
		// allocate immutable storage
		/*if (usage_ == ResourceUsage::Dynamic)
			gl::BufferStorage(bindingPoint, size, data, gl::DYNAMIC_STORAGE_BIT);
		else
			gl::BufferStorage(bindingPoint, size, data, 0);*/
		if (usage_ == ResourceUsage::Dynamic)
			gl::BufferData(bindingPoint, size, data, gl::STREAM_DRAW);
		else
			gl::BufferData(bindingPoint, size, data, gl::STATIC_DRAW);
		gl::BindBuffer(bindingPoint, 0);
	}

	GLuint id = -1;
	ResourceUsage usage;
	GLenum bindingPoint;
	std::size_t size;
};

 
#endif /* end of include guard: BUFFER_HPP */