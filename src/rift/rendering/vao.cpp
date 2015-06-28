#include <rendering/opengl4.hpp>

void VAO::create(unsigned num_buffers, util::array_ref<Attribute> attribs)
{
	strides.resize(num_buffers);
	std::fill(strides.begin(), strides.end(), 0);
	// create VAO
	gl::GenVertexArrays(1, &obj);
	if (!gl::exts::var_EXT_direct_state_access) {
		gl::BindVertexArray(obj);
	}
	int offset = 0;
	for (int attribindex = 0; attribindex < attribs.size(); ++attribindex)
	{
		const auto& attrib = attribs[attribindex];
		const auto& fmt = getElementFormatInfoGL(attrib.format);
		if (gl::exts::var_EXT_direct_state_access) {
			gl::EnableVertexArrayAttribEXT(
				obj,
				attribindex);
			gl::VertexArrayVertexAttribFormatEXT(
				obj,
				attribindex,
				fmt.size,
				fmt.type,
				fmt.normalize,
				strides[attrib.inputSlot]);
			gl::VertexArrayVertexAttribBindingEXT(
				obj,
				attribindex,
				attrib.inputSlot);
		}
		else {
			gl::EnableVertexAttribArray(
				attribindex);
			gl::VertexAttribFormat(
				attribindex,
				fmt.size,
				fmt.type,
				fmt.normalize,
				strides[attrib.inputSlot]);
			gl::VertexAttribBinding(
				attribindex,
				attrib.inputSlot);
		}
		strides[attrib.inputSlot] += getElementFormatSize(attrib.format);
	}
	if (!gl::exts::var_EXT_direct_state_access) {
		gl::BindVertexArray(0);
	}
}