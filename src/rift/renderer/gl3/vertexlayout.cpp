#include <vertexlayout.hpp>

VertexLayout::VertexLayout(std::array_ref<VertexElement2> elements_) :
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