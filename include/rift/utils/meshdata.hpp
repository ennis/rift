#include <glm/glm.hpp>

enum MDAT_LayoutType
{
	GL3_Layout_Full = 0,
	GL3_Layout_Packed = 1
};

// packed layout (less precise but more compact)
struct MDAT_Vertex
{
	glm::vec3 position;
	glm::uint32 normal; // packSnorm3x10_1x2
	glm::uint32 tangent; // packSnorm3x10_1x2
	glm::uint32 bitangent;	// packSnorm3x10_1x2
	glm::uint32 texcoord;	// packUnorm2x16
};


static const int MDAT_MaxSubMeshNameSize = 16;

// submesh descriptor
struct MDAT_SubMeshDesc
{
	uint32_t vertexOffset;
	uint32_t indexOffset;
	uint32_t numVertices;
	uint32_t numIndices;
	char name[MDAT_MaxSubMeshNameSize];
};

// file header
struct MDAT_Header
{
	char sig[4];	// 'MESH'
	uint16_t numSubMeshes;
	uint8_t vertexFormat;
	uint32_t dataOffset;
	uint32_t vertexSize;
	uint32_t indexSize;
};