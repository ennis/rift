#include <buffer.hpp>

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

Buffer::Buffer(std::size_t size_,
	ResourceUsage resourceUsage,
	BufferUsage bufferUsage,
	const void* initialData) :
	usage(resourceUsage), bindingPoint(bufferUsageToBindingPoint(bufferUsage)), size(size_)
{
	gl::GenBuffers(1, &id);
	gl::BindBuffer(bindingPoint, id);
	// allocate immutable storage
	if (resourceUsage == ResourceUsage::Dynamic)
		gl::BufferStorage(bindingPoint, size, initialData, gl::DYNAMIC_STORAGE_BIT);
	else
		gl::BufferStorage(bindingPoint, size, initialData, 0);
	/*if (usage_ == ResourceUsage::Dynamic)
	gl::BufferData(bindingPoint, size, data, gl::STREAM_DRAW);
	else
	gl::BufferData(bindingPoint, size, data, gl::STATIC_DRAW);*/
	gl::BindBuffer(bindingPoint, 0);
}


void Buffer::update(std::size_t offset, std::size_t size, const void *data)
{
	if (gl::exts::var_EXT_direct_state_access) {
		gl::NamedBufferSubDataEXT(id, offset, size, data);
	}
	else {
		gl::BindBuffer(bindingPoint, id);
		gl::BufferSubData(bindingPoint, offset, size, data);
	}
}
