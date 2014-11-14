#include <common.hpp>
#include <vector>

namespace Importer
{
	class Model
	{
	public:
		struct Vertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;
			glm::vec2 texcoord;
			glm::i8vec4 boneIds;
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
		};

		Model(const char *filePath);
		void export(std::ostream &streamOut);

	private:
		void import(const char *filePath);
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Bone> bones;
		std::vector<Submesh> submeshes;
	};
}