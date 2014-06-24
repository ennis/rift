#ifndef MESHBUFFER_HPP
#define MESHBUFFER_HPP

#include <common.hpp>
#include <renderresource.hpp>

//
//
enum class PrimitiveType
{
	Points,
	Lines,
	Triangles
};

enum VertexLayoutType
{
	Layout_Full,
	Layout_Position,
	Layout_Packed,
	Layout_MaxLayoutType
};

struct MeshBufferDesc
{
	MeshBufferDesc() = default;
	PrimitiveType primitiveType = PrimitiveType::Points;
	VertexLayoutType layoutType = Layout_Full;
	std::size_t numVertices = 0;
	std::size_t numIndices = 0;
};

struct MeshBufferInit
{
	MeshBufferInit() = default;

	MeshBufferDesc desc;
	glm::vec3 const *positions = nullptr;
	glm::vec3 const *normals = nullptr;
	glm::vec2 const *texcoords = nullptr;
	glm::vec4 const *colors = nullptr;
	uint16_t const *indices = nullptr;
};


//
// Full
struct VertexLayoutFull
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
	glm::vec4 color;
};

//
// Packed layout
struct VertexLayoutPacked
{
	glm::vec3 position;
	uint32_t align;		// dummy
	uint32_t normal;	// int_2_10_10_10
	glm::i16vec2 texcoord;
	glm::u8vec4 color;
};

struct CMeshBuffer : public CRenderResource
{
	MeshBufferDesc mDesc;

	virtual void updateVertex(void const *vertexData, std::size_t offset, std::size_t size) = 0;
	virtual void updateIndex(void const *vertexData, std::size_t offset, std::size_t size) = 0;
};

#endif