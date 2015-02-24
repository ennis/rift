#ifndef CONSTANT_BUFFER_HPP
#define CONSTANT_BUFFER_HPP

#include <renderer_common.hpp>


template <typename Backend>
class ConstantBufferBase : 
	public RendererObject<typename Backend::ConstantBufferImpl>
{
public:
	// nullable
	ConstantBufferBase() = default;
	// ctor
	ConstantBufferBase(Impl impl_) : 
		RendererObject<
			typename Backend::ConstantBufferImpl
		>
		(impl_)
	{}
	// noncopyable
	ConstantBufferBase(const ConstantBufferBase<Backend> &) = delete;
	ConstantBufferBase<Backend> &operator=(const ConstantBufferBase<Backend> &) = delete;
	// moveable
	ConstantBufferBase(ConstantBufferBase<Backend> &&rhs) {
		*this = std::move(rhs);
	}
	ConstantBufferBase &operator=(ConstantBufferBase<Backend> &&rhs) {
		std::swap(impl, rhs.impl);
		return *this;
	}

	~ConstantBufferBase() {
		Backend::getInstance().deleteConstantBuffer(impl);
	}

	ConstantBufferBase(int size, const void *initialData = nullptr) : 
		RendererObject<
			typename Backend::ConstantBufferImpl
		>
		(Backend::getInstance().createConstantBuffer(size, initialData))
	{
	}

	//=========================================
	void update(
		int offset, 
		int size, 
		const void *data)
	{
		Backend::getInstance().updateConstantBuffer(impl, offset, size, data);
	}

};

 

 
#endif /* end of include guard: CONSTANT_BUFFER_HPP */