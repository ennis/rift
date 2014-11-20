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

}}

// TODO load model hints
// ModelHint::ShadowPosition
// ModelHint::ShadowBuffer
// ModelHint::Mutable
void Model::loadFromFile(const char *filePath)
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
	// vertex data
	unpacker.unpack_bin(mVertexData, mVertexDataSize);
	unpacker.unpack_bin(mIndexData, mIndexDataSize);
}