#ifndef VERTEXLAYOUT_HPP
#define VERTEXLAYOUT_HPP

#include <renderer_common.hpp>

template <typename Backend>
class VertexLayout
{
public:
	typedef typename Backend::VertexLayoutImpl Impl;
	VertexLayout() = default;
	VertexLayout(VertexLayoutImpl impl);
	VertexLayout(const VertexLayout<Backend>&) = delete;
	VertexLayout &operator=(const VertexLayout<Backend>&) = delete;
	VertexLayout(VertexLayout<Backend> &&rhs) {
		*this = std::move(rhs);
	}
	VertexLayout &operator=(VertexLayout<Backend> &&rhs) {
		std::swap(impl, rhs.impl);
		return *this;
	}
	~VertexLayout();

	VertexLayout(std::array_ref<VertexElement2> elements_);

private:
	Impl impl;
};

 
#endif /* end of include guard: VERTEXLAYOUT_HPP */