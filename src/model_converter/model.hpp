#include <common.hpp>
#include <vector>

namespace Importer
{
	class Model
	{
	public:
		// TODO sync with librift model class
		struct Vertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;
			glm::vec2 texcoord;
			glm::u8vec4 boneIds;
			glm::vec4 boneWeights;
		};

		struct Bone {
			Bone() = default;
			char name[32];
			glm::mat4 invBindPose;
			glm::mat4 transform;
			int parent;
		};

		struct Submesh {
			unsigned int startVertex;
			unsigned int startIndex;
			unsigned int numVertices;
			unsigned int numIndices;
			int bone;
		};

		Model(const char *filePath);
		void exportModel(std::ostream &streamOut);

	private:
		void import(const char *filePath);
		bool isSkinned;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;
		std::vector<glm::vec2> texcoords;
		std::vector<glm::u8vec4> boneIds;
		std::vector<glm::vec4> boneWeights;
		std::vector<uint32_t> indices;
		std::vector<Bone> bones;
		std::vector<Submesh> submeshes;
	};
}
