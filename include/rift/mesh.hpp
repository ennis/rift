#ifndef MESH_HPP
#define MESH_HPP

#include <gl4/renderer.hpp>

class Mesh
{
public:
	using Ptr = std::unique_ptr < Mesh > ;

	Mesh(util::array_ref<Attribute> layout,
		int numVertices,
		const void *vertexData,
		int numIndices,
		const void *indexData,
		util::array_ref<Submesh> submeshes);

	// VS2013
	/*Mesh(Mesh &&rhs) :
		mode(rhs.mode),
		vb(std::move(rhs.vb)),
		ib(std::move(rhs.ib)),
		vao(std::move(rhs.vao)),
		vbsize(rhs.vbsize),
		ibsize(rhs.ibsize),
		nbvertex(rhs.nbvertex),
		nbindex(rhs.nbindex),
		stride(rhs.stride),
		index_format(rhs.index_format),
		submeshes(std::move(rhs.submeshes))
	{}
	Mesh &operator=(Mesh &&rhs) {
		mode = rhs.mode;
		vb = std::move(rhs.vb);
		ib = std::move(rhs.ib);
		vao = std::move(rhs.vao);
		vbsize = rhs.vbsize;
		ibsize = rhs.ibsize;
		nbvertex = rhs.nbvertex;
		nbindex = rhs.nbindex;
		stride = rhs.stride;
		index_format = rhs.index_format;
		submeshes = std::move(rhs.submeshes);
		return *this;
	}*/
	// -VS2013

//protected:
	Mesh() = default;

	static Ptr create(
		util::array_ref<Attribute> layout,
		int numVertices,
		const void *vertexData,
		int numIndices,
		const void *indexData,
		util::array_ref<Submesh> submeshes)
	{
		return std::make_unique<Mesh>(
			layout, 
			numVertices, 
			vertexData, 
			numIndices, 
			indexData, 
			submeshes);
	}

	void draw(RenderQueue &renderQueue, unsigned submesh);

	static Ptr loadFromArchive(serialization::IArchive &ar);

	InputLayout::Ptr layout;
	Buffer::Ptr vbo;
	Buffer::Ptr ibo;
	int nbvertex;
	int nbindex;
	int stride;
	GLenum index_format;
	std::vector<Submesh> submeshes;
};
 
#endif /* end of include guard: MESH_HPP */