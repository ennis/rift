#include <model.hpp>
#include <serialization.hpp>

Model::Model(Renderer &renderer, const char *filePath, unsigned int hints) : 
	Model(Model::loadFromFile(renderer, filePath, hints))
{
}

Model::Model(Model &&rhs) : 
mRenderer(rhs.mRenderer),
mMesh(rhs.mRenderer)
{
	*this = std::move(rhs);
}

Model::Model(
	Renderer &renderer,
	std::vector<Submesh> &&submeshes,
	std::vector<uint16_t> &&indices,
	std::vector<Bone> &&bones,
	std::vector<glm::vec3> &&positions,
	std::vector<glm::vec3> &&normals,
	std::vector<glm::vec3> &&tangents,
	std::vector<glm::vec3> &&bitangents,
	std::vector<glm::vec2> &&texcoords,
	std::vector<glm::u8vec4> &&boneIds,
	std::vector<glm::vec4> &&boneWeights) :
mRenderer(renderer),
mSubmeshes(submeshes),
mIndices(std::move(indices)),
mBones(std::move(bones)),
mMesh(renderer),
mPositions(std::move(positions)),
mNormals(std::move(normals)),
mTangents(std::move(tangents)),
mBitangents(std::move(bitangents)),
mTexcoords(std::move(texcoords)),
mBoneIDs(std::move(boneIds)),
mBoneWeights(std::move(boneWeights))
{
}

Model::~Model()
{
}

Model &Model::operator=(Model &&rhs)
{
	mSubmeshes = std::move(rhs.mSubmeshes);
	mIndices = std::move(rhs.mIndices);
	mBones = std::move(rhs.mBones);
	mMesh = std::move(rhs.mMesh);
	mPositions = std::move(rhs.mPositions);
	mNormals = std::move(rhs.mNormals);
	mTangents = std::move(rhs.mTangents);
	mBitangents = std::move(rhs.mBitangents);
	mTexcoords = std::move(rhs.mTexcoords);
	mBoneIDs = std::move(rhs.mBoneIDs);
	mBoneWeights = std::move(rhs.mBoneWeights);
	return *this;
}

namespace rift {
namespace serialization {

	// glm::mat4 unpacker
	template <>
	struct pack_traits < glm::mat4 > {
		static void unpack(Unpacker &u, glm::mat4 &v) {
			for (unsigned int i = 0; i < 4; ++i) {
				for (unsigned int j = 0; j < 4; ++j) {
					u.unpack(v[i][j]);
				}
			}
		}
	};

	// glm::vecX unpacker
	template <>
	struct pack_traits < glm::vec2 > {
		static void unpack(Unpacker &u, glm::vec2 &v) {
			u.unpack(v[0]);
			u.unpack(v[1]);
		}
	};

	template <>
	struct pack_traits < glm::vec3 > {
		static void unpack(Unpacker &u, glm::vec3 &v) {
			u.unpack(v[0]);
			u.unpack(v[1]);
			u.unpack(v[2]);
		}
	};

	template <>
	struct pack_traits < glm::vec4 > {
		static void unpack(Unpacker &u, glm::vec4 &v) {
			u.unpack(v[0]);
			u.unpack(v[1]);
			u.unpack(v[2]);
			u.unpack(v[3]);
		}
	};

	template <>
	struct pack_traits < glm::u8vec4 > {
		static void unpack(Unpacker &u, glm::u8vec4 &v) {
			char ch[4];
			u.unpack_n(ch);
			v = glm::u8vec4(ch[0], ch[1], ch[2], ch[3]);
		}
	};

}}


Model Model::loadFromFile(Renderer &renderer, const char *filePath, unsigned int hints)
{
	using namespace rift::serialization;

	std::ifstream fileIn(filePath, std::ios::in | std::ios::binary);
	Unpacker unpacker(fileIn);

	unsigned int numSubmeshes, numBones, numVertices, numIndices, format;
	std::vector<Model::Submesh> submeshes;
	std::vector<Model::Bone> bones;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	std::vector<glm::vec2> texcoords;
	std::vector<glm::u8vec4> boneIds;
	std::vector<glm::vec4> boneWeights;
	std::vector<uint16_t> indices;
	
	unpacker.unpack(numSubmeshes);
	assert(numSubmeshes < 65536);
	submeshes.reserve(numSubmeshes);
	for (unsigned int i = 0; i < numSubmeshes; ++i) {
		Submesh sm;
		unpacker.unpack(sm.startVertex);
		unpacker.unpack(sm.startIndex);
		unpacker.unpack(sm.numVertices);
		unpacker.unpack(sm.numIndices);
		unpacker.unpack(sm.bone);
		submeshes.push_back(sm);
	}

	unpacker.unpack(numBones);
	assert(numBones < 256);
	bones.reserve(numBones);
	for (unsigned int i = 0; i < numBones; ++i) {
		Bone b;
		std::string name;
		unpacker.unpack(name);
		unpacker.unpack(b.transform);
		unpacker.unpack(b.invBindPose);
		unpacker.unpack(b.parent);
		bones.push_back(b);
	}

	unpacker.unpack(numVertices);
	unpacker.unpack(numIndices);
	// TODO what is a reasonable size?
	assert(numVertices < 40*1024*1024);
	assert(numIndices < 40*1024*1024);
	unpacker.unpack(format);

	positions.reserve(numVertices);
	normals.reserve(numVertices);
	tangents.reserve(numVertices);
	bitangents.reserve(numVertices);
	texcoords.reserve(numVertices);
	boneIds.reserve(numVertices);
	boneWeights.reserve(numVertices);
	indices.reserve(numIndices);

	// unpack data
	// type 0/1 - non-interleaved attributes
	if (format == 0 || format == 1)
	{
		// ==== positions ====
		for (unsigned int i = 0; i < numVertices; ++i) {
			glm::vec3 v;
			unpacker.unpack(v);
			positions.push_back(v);
		}

		// ==== normals ====
		for (unsigned int i = 0; i < numVertices; ++i) {
			glm::vec3 v;
			unpacker.unpack(v);
			normals.push_back(v);
		}

		// ==== tangents ====
		for (unsigned int i = 0; i < numVertices; ++i) {
			glm::vec3 v;
			unpacker.unpack(v);
			tangents.push_back(v);
		}

		// ==== bitangents ====
		for (unsigned int i = 0; i < numVertices; ++i) {
			glm::vec3 v;
			unpacker.unpack(v);
			bitangents.push_back(v);
		}

		// ==== texcoords ====
		for (unsigned int i = 0; i < numVertices; ++i) {
			glm::vec2 v;
			unpacker.unpack(v);
			texcoords.push_back(v);
		}

		if (format == 1) {
			// ==== bone ids ====
			for (unsigned int i = 0; i < numVertices; ++i) {
				glm::u8vec4 v;
				unpacker.unpack(v);
				boneIds.push_back(v);
			}

			// ==== bone weights ====
			for (unsigned int i = 0; i < numVertices; ++i) {
				glm::vec4 v;
				unpacker.unpack(v);
				boneWeights.push_back(v);
			}
		}
		for (unsigned int i = 0; i < numIndices; ++i) {
			unsigned short ix;
			unpacker.unpack16(ix);
			indices.push_back(ix);
		}
	}
	else if (format == 2) {
		// type 2 - packed attributes - static mesh
		// TODO
	}
	else if (format == 3) {
		// type 3 - packed attributes - skinned mesh
		// TODO
	}

	// build bone tree
	auto nb = bones.size();
	for (unsigned int i = 0; i < nb; ++i) {
		int p = bones[i].parent;
		if (p != -1) {
			assert(p < nb);
			bones[p].children.push_back(i);
		}
	}

	return Model(
		renderer,
		std::move(submeshes),
		std::move(indices),
		std::move(bones),
		std::move(positions),
		std::move(normals),
		std::move(tangents),
		std::move(bitangents),
		std::move(texcoords),
		std::move(boneIds),
		std::move(boneWeights));
}