#ifndef VERTEXLAYOUT_HPP
#define VERTEXLAYOUT_HPP

#include <gl_common.hpp>
#include <array_ref.hpp>
#include <utility>	// move

class VertexLayout
{
public:
	friend class Renderer;
	VertexLayout() = default;
	VertexLayout(VertexLayout &&rhs) {
		*this = std::move(rhs);
	}
	VertexLayout &operator=(VertexLayout&& rhs) {
		vao = rhs.vao;
		elements = std::move(rhs.elements);
		rhs.vao = 0;
		return *this;
	}
	VertexLayout(const VertexLayout&) = delete;
	VertexLayout &operator=(const VertexLayout&) = delete;

	VertexLayout(std::array_ref<VertexElement2> elements_);

	~VertexLayout()
	{
		if (vao) {
			gl::DeleteVertexArrays(1, &vao);
		}
	}

	bool isNull() const {
		return vao == 0;
	}

private:
	std::vector<VertexElement2> elements;
	GLuint vao = 0;
};

 
#endif /* end of include guard: VERTEXLAYOUT_HPP */