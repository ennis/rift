#ifndef Mesh_HPP
#define Mesh_HPP

#include <renderer2.hpp>
#include <array>
#include <array_ref.hpp>

class Mesh
{
public:
	struct Attribute {
		int inputSlot;
		ElementFormat elementFormat;
	};

	struct BufferDesc {
		ResourceUsage usage;
	};

	Mesh() = default;
	Mesh(
		PrimitiveType primitiveType,
		std::array_ref<Mesh::Attribute> attributes,
		std::array_ref<Mesh::BufferDesc> buffers,
		unsigned int numVertices,
		const void *initialVertices[],
		unsigned int numIndices,
		ElementFormat indexFormat,
		ResourceUsage indexUsage,
		const void *initialIndices);

	Mesh(Mesh const &rhs) = delete;
	Mesh(Mesh &&rhs);
	~Mesh();
	Mesh &operator=(Mesh &&rhs);

	void allocate(
		PrimitiveType primitiveType,
		std::array_ref<Mesh::Attribute> attributes,
		std::array_ref<Mesh::BufferDesc> buffers,
		unsigned int numVertices,
		const void *initialVertices[],
		unsigned int numIndices,
		ElementFormat indexFormat,
		ResourceUsage indexUsage,
		const void *initialIndices);

	void update(
		unsigned int buffer,
		unsigned int startVertex,
		unsigned int numVertices, 
		const void *data);

	void updateIndices(
		unsigned int startIndex, 
		unsigned int numVertices,
		const void *data);

	std::array_ref<Buffer> getVertexBuffers() const {
		return mVertexBuffers;
	}
	const Buffer &getIndexBuffer() const {
		return mIndexBuffer;
	}
	const VertexLayout &getVertexLayout() const {
		return mVertexLayout;
	}
	PrimitiveType getPrimitiveType() const {
		return mPrimitiveType;
	}

	void draw(
		Renderer &renderer
		) const;

	void drawPart(
		Renderer &renderer,
		int baseVertex,
		int numVertices
		) const;

	void drawPart(
		Renderer &renderer,
		int baseVertex,
		int startIndex, 
		int numIndices
		) const;

	unsigned int getNumVertices() const {
		return mNumVertices;
	}

	unsigned int getNumIndices() const {
		return mNumIndices;
	}

private:
	void prepareDraw(Renderer &renderer) const;

	std::vector<Buffer> mVertexBuffers;	
	Buffer mIndexBuffer;	
	VertexLayout mVertexLayout;	
	PrimitiveType mPrimitiveType;
	std::array<int, 16> mStrides;
	unsigned int mIndexStride = 0;
	unsigned int mNumBuffers = 0;
	unsigned int mNumVertices = 0;
	unsigned int mNumIndices = 0;
};

 
#endif /* end of include guard: Mesh_HPP */