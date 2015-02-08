#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <gl_common.hpp>
#include <utility>	// move

class Buffer 
{
public:
	friend class Renderer;
	Buffer() = default;
	Buffer(Buffer &&rhs) {
		*this = std::move(rhs);
	}
	Buffer &operator=(Buffer&& rhs) {
		id = rhs.id;
		usage = rhs.usage;
		size = rhs.size;
		bindingPoint = rhs.bindingPoint;
		// reset object
		rhs.id = 0;
		rhs.size = 0;
		return *this;
	}
	Buffer(const Buffer&) = delete;
	Buffer &operator=(const Buffer&) = delete;

	Buffer::~Buffer()
	{
		if (id) {
			gl::DeleteBuffers(1, &id);
		}
	}

	void update(
		std::size_t offset, 
		std::size_t size, 
		const void *data
		);

	Buffer(
		std::size_t size,
		ResourceUsage resourceUsage,
		BufferUsage bufferUsage,
		const void* initialData
		);

	bool isNull() const {
		return id == 0;
	}

private:
	GLuint id = 0;
	ResourceUsage usage;
	GLenum bindingPoint;
	std::size_t size;
};

 
#endif /* end of include guard: BUFFER_HPP */