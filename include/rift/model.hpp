#ifndef MODEL_HPP
#define MODEL_HPP

#include <renderer.hpp>
#include <mesh.hpp>

// static models
class Model
{
public:
	struct GPUVertex {
		// packed layout (less precise but more compact)
		glm::vec3 position;
		glm::uint32 normal; // packSnorm3x10_1x2
		glm::uint32 tangent; // packSnorm3x10_1x2
		glm::uint32 bitangent;	// packSnorm3x10_1x2
		glm::uint32 texcoord;	// packUnorm2x16
	}; 
	
	struct GPUSkinnedVertex {
		glm::vec3 position;
		glm::uint32 normal; // packSnorm3x10_1x2
		glm::uint32 tangent; // packSnorm3x10_1x2
		glm::uint32 bitangent;	// packSnorm3x10_1x2
		glm::uint32 texcoord;	// packUnorm2x16
		glm::u8vec4 boneIds;
		glm::uint64 boneWeights; // packUnorm4x16
	};

	struct Bone {
		int parent;
		glm::mat4 transform;
		glm::mat4 invBindPose;
		glm::mat4 finalTransform;
	};

	struct GPUBone {
		glm::mat4 transform;
	};

	struct Submesh {
		unsigned int startVertex;
		unsigned int startIndex;
		unsigned int numVertices;
		unsigned int numIndices;
	};

	Model(Renderer &renderer);
	Model(Renderer &renderer, const char *filePath);
	~Model();
	
	void loadFromFile(const char *filePath);

	uint8_t *getVertexData() const {
		// TODO type-safe
		return mVertexData.get();
	}
	uint8_t *getIndexData() const {
		// TODO type-safe
		return mIndexData.get();
	}

	unsigned int numVertices() const {
		return mNumVertices;
	}
	unsigned int numIndices() const {
		return mNumIndices;
	}
	
private:
	Renderer &mRenderer;
	std::vector<Submesh> mSubmeshes;
	//std::vector<Model::Vertex> mVertices;
	std::vector<uint32_t> mIndices;
	std::vector<Bone> mBones;
	std::unique_ptr<Mesh> mMesh;
	unsigned int mVertexDataSize;
	unsigned int mIndexDataSize;
	std::unique_ptr<uint8_t[]> mVertexData;
	std::unique_ptr<uint8_t[]> mIndexData;
	unsigned int mNumVertices;
	unsigned int mNumIndices;
};

 
#endif /* end of include guard: MODEL_HPP */