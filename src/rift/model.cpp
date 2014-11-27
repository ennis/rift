#include <model.hpp>
#include <serialization.hpp>

Model::Model(Renderer &renderer) : mRenderer(renderer)
{
}

Model::Model(Renderer &renderer, const char *filePath) : mRenderer(renderer)
{
	loadFromFile(filePath);
}

Model::~Model()
{
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

void Model::loadFromFile(const char *filePath, unsigned int hints)
{
	using namespace rift::serialization;

	std::ifstream fileIn(filePath, std::ios::in | std::ios::binary);
	Unpacker unpacker(fileIn);

	unpacker.unpack(mSubmeshes, [](Unpacker &u) -> Submesh {
		Submesh sm;
		u.unpack(sm.startVertex);
		u.unpack(sm.startIndex);
		u.unpack(sm.numVertices);
		u.unpack(sm.numIndices);
		u.unpack(sm.bone);
		return sm;
	});
	// skip bones
	unpacker.unpack(mBones, [](Unpacker &u) -> Bone {
		Bone b;
		std::string name;
		u.unpack(name);
		u.unpack(b.transform);
		u.unpack(b.invBindPose);
		u.unpack(b.parent);
		return b;
	});
	unpacker.unpack(mNumVertices);
	unpacker.unpack(mNumIndices);
	unsigned int format;
	unpacker.unpack(format);

	// unpack data
	// type 0/1 - non-interleaved attributes
	if (format == 0 || format == 1)
	{
		unpacker.unpack_n(mNumVertices, mPositions);
		unpacker.unpack_n(mNumVertices, mNormals);
		unpacker.unpack_n(mNumVertices, mTangents);
		unpacker.unpack_n(mNumVertices, mBitangents);
		unpacker.unpack_n(mNumVertices, mTexcoords);
		if (format == 1) {
			unpacker.unpack_n(mNumVertices, mBoneIDs);
			unpacker.unpack_n(mNumVertices, mBoneWeights);
		}
		for (unsigned int i = 0; i < mNumIndices; ++i) {
			unsigned short ix;
			unpacker.unpack16(ix);
			mIndices.push_back(ix);
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
	auto nb = mBones.size();
	for (unsigned int i = 0; i < nb; ++i) {
		int p = mBones[i].parent;
		if (p != -1) {
			assert(p < nb);
			mBones[p].children.push_back(i);
		}
	}
}