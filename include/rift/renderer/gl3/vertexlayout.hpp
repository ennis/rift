#ifndef VERTEXLAYOUT_HPP
#define VERTEXLAYOUT_HPP

#include <globject.hpp>
#include <array_ref.hpp>

class VertexLayout : public GLAPIObject
{
public:
	GL_MOVEABLE_OBJECT_IMPL(VertexLayout)
	GL_IS_NULL_IMPL(vao)

	VertexLayout(std::array_ref<VertexElement2> elements_) :
	elements(elements_.vec())
	{
		// create VAO
		gl::GenVertexArrays(1, &vao);
		if (!gl::exts::var_EXT_direct_state_access) {
			gl::BindVertexArray(vao);
		}
		for (unsigned int attribindex = 0; attribindex < elements.size(); ++attribindex)
		{
			const auto& attrib = elements[attribindex];
			const auto& fmt = getElementFormatInfoGL(attrib.format);
			if (gl::exts::var_EXT_direct_state_access) {
				gl::EnableVertexArrayAttribEXT(vao, attribindex);
				gl::VertexArrayVertexAttribFormatEXT(vao, attribindex, fmt.size, fmt.type, fmt.normalize, attrib.offset);
				gl::VertexArrayVertexAttribBindingEXT(vao, attribindex, attrib.inputSlot);
			}
			else {
				gl::EnableVertexAttribArray(attribindex);
				gl::VertexAttribFormat(attribindex, fmt.size, fmt.type, fmt.normalize, attrib.offset);
				gl::VertexAttribBinding(attribindex, attrib.inputSlot);
			}
		}
		if (!gl::exts::var_EXT_direct_state_access) {
			gl::BindVertexArray(0);
		}
	}

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
	std::vector<VertexElement2> elements;
	GLuint vao = 0;
};

 
#endif /* end of include guard: VERTEXLAYOUT_HPP */