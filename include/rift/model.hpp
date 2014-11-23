#ifndef MODEL_HPP
#define MODEL_HPP

#include <renderer.hpp>
#include <mesh.hpp>

namespace ModelLoadHint
{
	static const unsigned int Unpack = (1 << 0);
	static const unsigned int Static = (1 << 1);
};

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
		// TODO SmallVector
		std::vector<int> children;
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
	
	void loadFromFile(const char *filePath, unsigned int hints = 0);

	unsigned int numVertices() const {
		return mNumVertices;
	}
	unsigned int numIndices() const {
		return mNumIndices;
	}
	std::vector<Bone> const &getBones() const {
		return mBones;
	}
	std::vector<glm::vec3> const &getPositions() const {
		return mPositions;
	}
	std::vector<glm::vec3> const &getNormals() const {
		return mNormals;
	}
	std::vector<glm::vec3> const &getTangents() const {
		return mTangents;
	}
	std::vector<glm::vec3> const &getBitangents() const {
		return mBitangents;
	}
	std::vector<glm::vec2> const &getTexcoords(unsigned int id = 0) const {
		return mTexcoords;
	}
	std::vector<glm::u8vec4> const &getBoneIndices() const {
		return mBoneIDs;
	}
	std::vector<glm::vec4> const &getBoneWeights() const {
		return mBoneWeights;
	}
	std::vector<uint16_t> const &getIndices() const {
		return mIndices;
	}
	std::vector<Model::Submesh> const &getSubmeshes() const {
		return mSubmeshes;
	}
	
private:
	Renderer &mRenderer;
	std::vector<Submesh> mSubmeshes;
	//std::vector<Model::Vertex> mVertices;
	std::vector<uint16_t> mIndices;
	std::vector<Bone> mBones;
	std::unique_ptr<Mesh> mMesh;
	unsigned int mNumVertices;
	unsigned int mNumIndices;
	std::vector<glm::vec3> mPositions;
	std::vector<glm::vec3> mNormals;
	std::vector<glm::vec3> mTangents;
	std::vector<glm::vec3> mBitangents;
	std::vector<glm::vec2> mTexcoords;
	std::vector<glm::u8vec4> mBoneIDs;
	std::vector<glm::vec4> mBoneWeights;
};

 
#endif /* end of include guard: MODEL_HPP */