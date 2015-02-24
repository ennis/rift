#ifndef MESH_HPP
#define MESH_HPP

#include <renderer_common.hpp>
#include <array_ref.hpp>

template <typename Backend>
class MeshBase : public RendererObject<typename Backend::MeshImpl>
{
public:
	// nullable
	MeshBase() = default;
	// ctor
	MeshBase(Impl impl_) : RendererObject<typename Backend::MeshImpl>(impl_)
	{}
	// noncopyable
	MeshBase(const MeshBase<Backend> &) = delete;
	MeshBase<Backend> &operator=(const MeshBase<Backend> &) = delete;
	// moveable
	MeshBase(MeshBase<Backend> &&rhs) {
		*this = std::move(rhs);
	}
	MeshBase &operator=(MeshBase<Backend> &&rhs) {
		std::swap(impl, rhs.impl);
		return *this;
	}
	~MeshBase() {
		Backend::getInstance().deleteMesh(impl);
	}

	MeshBase(
		PrimitiveType primitiveType,
		std::array_ref<Attribute> layout,
		int numVertices,
		const void *vertexData,
		int numIndices,
		const void *indexData,
		std::array_ref<Submesh> submeshes)
	{
		impl = Backend::getInstance().createMesh(
			primitiveType,
			layout,
			numVertices,
			vertexData,
			numIndices,
			indexData,
			submeshes
			);
	}
};

 
#endif /* end of include guard: MESH_HPP */