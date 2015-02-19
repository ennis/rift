#include <model.hpp>
#include <serialization.hpp>
#include <glm/gtc/packing.hpp>
#include <fstream>

Model::Model(Renderer &renderer, const char *filePath, unsigned int hints) 
{
	loadFromFile(renderer, filePath, hints);
}

Model::Model(Model &&rhs)
{
	*this = std::move(rhs);
}

Model::~Model()
{
}

Model &Model::operator=(Model &&rhs)
{
	mRenderer = std::move(rhs.mRenderer);
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

void Model::optimize()
{
	using namespace glm;

	// TODO skinning
	Mesh::BufferDesc buf[] = { { ResourceUsage::Static } };
	Mesh::Attribute attribs[] = {
		{ 0, ElementFormat::Float3 },
		{ 0, ElementFormat::Snorm10x3_1x2 },
		{ 0, ElementFormat::Snorm10x3_1x2 },
		{ 0, ElementFormat::Snorm10x3_1x2 },
		{ 0, ElementFormat::Unorm16x2 },
	};

	auto nv = getNumVertices();

	std::vector<GPUVertex> data;
	data.reserve(nv);

	for (unsigned int i = 0; i < nv; ++i)
	{
		data.push_back(GPUVertex{
			mPositions[i],
			packSnorm3x10_1x2(vec4(mNormals[i], 0.0f)),
			packSnorm3x10_1x2(vec4(mTangents[i], 0.0f)),
			packSnorm3x10_1x2(vec4(mBitangents[i], 0.0f)),
			packUnorm2x16(mTexcoords[i])
		});
	}

	const void *ptr = data.data();

		// create optimized mesh
	mMesh.allocate(
		PrimitiveType::Triangle,
		attribs,
		buf,
		nv,
		&ptr,
		getNumIndices(),
		ElementFormat::Uint16,
		ResourceUsage::Static,
		mIndices.data());
}

const Mesh &Model::getMesh() const
{
	return mMesh;
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


void Model::loadFromFile(Renderer &renderer, const char *filePath, unsigned int hints)
{
	using namespace rift::serialization;

	mRenderer = &renderer;

	std::ifstream fileIn(filePath, std::ios::in | std::ios::binary);
	Unpacker unpacker(fileIn);

	unsigned int numSubmeshes, numBones, numVertices, numIndices, format;
	unpacker.unpack(numSubmeshes);
	assert(numSubmeshes < 65536);
	mSubmeshes.reserve(numSubmeshes);
	for (unsigned int i = 0; i < numSubmeshes; ++i) {
		Submesh sm;
		unpacker.unpack(sm.startVertex);
		unpacker.unpack(sm.startIndex);
		unpacker.unpack(sm.numVertices);
		read_u16le(fileIn, sm.numIndices);
		read_u8(fileIn, sm.bone);
		mSubmeshes.push_back(sm);
	}

	read_u8(fileIn, numBones);
	assert(numBones < 256);
	mBones.reserve(numBones);
	for (unsigned int i = 0; i < numBones; ++i) {
		Bone b;
		std::string name;
		unpacker.unpack(name);
		unpacker.unpack(b.transform);
		unpacker.unpack(b.invBindPose); 
		read_u8(fileIn, b.parent);
		mBones.push_back(b);
	}

	unpacker.unpack(numVertices);
	unpacker.unpack(numIndices);
	// TODO what is a reasonable size?
	assert(numVertices < 40*1024*1024);
	assert(numIndices < 40*1024*1024);
	read_u8(fileIn, format);

	mPositions.reserve(numVertices);
	mNormals.reserve(numVertices);
	mTangents.reserve(numVertices);
	mBitangents.reserve(numVertices);
	mTexcoords.reserve(numVertices);
	mBoneIDs.reserve(numVertices);
	mBoneWeights.reserve(numVertices);
	mIndices.reserve(numIndices);

	// unpack data
	// type 0/1 - non-interleaved attributes
	if (format == 0 || format == 1)
	{
		// ==== mPositions ====
		for (unsigned int i = 0; i < numVertices; ++i) {
			glm::vec3 v;
			unpacker.unpack(v);
			mPositions.push_back(v);
		}

		// ==== mNormals ====
		for (unsigned int i = 0; i < numVertices; ++i) {
			glm::vec3 v;
			unpacker.unpack(v);
			mNormals.push_back(v);
		}

		// ==== mTangents ====
		for (unsigned int i = 0; i < numVertices; ++i) {
			glm::vec3 v;
			unpacker.unpack(v);
			mTangents.push_back(v);
		}

		// ==== bitangents ====
		for (unsigned int i = 0; i < numVertices; ++i) {
			glm::vec3 v;
			unpacker.unpack(v);
			mBitangents.push_back(v);
		}

		// ==== mTexcoords ====
		for (unsigned int i = 0; i < numVertices; ++i) {
			glm::vec2 v;
			unpacker.unpack(v);
			mTexcoords.push_back(v);
		}

		if (format == 1) {
			// ==== bone ids ====
			for (unsigned int i = 0; i < numVertices; ++i) {
				glm::u8vec4 v;
				unpacker.unpack(v);
				mBoneIDs.push_back(v);
			}

			// ==== bone weights ====
			for (unsigned int i = 0; i < numVertices; ++i) {
				glm::vec4 v;
				unpacker.unpack(v);
				mBoneWeights.push_back(v);
			}
		}
		for (unsigned int i = 0; i < numIndices; ++i) {
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
		// p == 255 means root node
		if (p != 255) {
			assert(p < nb);
			mBones[p].children.push_back(i);
		}
	}
}