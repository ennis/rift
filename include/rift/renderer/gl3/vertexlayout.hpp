#ifndef VERTEXLAYOUT_HPP
#define VERTEXLAYOUT_HPP

#include <globject.hpp>

class VertexLayout : public GLAPIObject
{
public:
	GL_MOVEABLE_OBJECT_IMPL(VertexLayout)
	GL_IS_NULL_IMPL(vao)

	void swap(VertexLayout &&rhs)
	{
		std::swap(vao, rhs.vao);
	}

	~VertexLayout()
	{
		if (vao) {
			gl::DeleteVertexArrays(1, &vao);
		}
	}

private:
	VertexLayout(GLuint vao_) : vao(vao_)
	{}

	GLuint vao;
};

 
#endif /* end of include guard: VERTEXLAYOUT_HPP */