#include <model.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>
#include <serialization.hpp>
#include <unordered_set>
#include <renderer.hpp>
#include <cassert>
#include <msgpack\msgpack.hpp>
#include <msgpack\msgpack_fwd.hpp>

namespace 
{
	aiNode *findNodeWithName(aiNode *root,aiString &name)
	{
		if (root->mName==name) {
			return root;
		}
		for (unsigned int ic=0; ic<root->mNumChildren; ++ic) {
			auto f=findNodeWithName(root->mChildren[ic],name);
			if (f) return f;
		}
		return nullptr;
	}

	void addBranch(std::unordered_set<aiNode*> &map,aiNode *node)
	{
		if (node!=nullptr) {
			map.insert(node);
			addBranch(map,node->mParent);
		}
	}

	void writeSubmesh(BinaryTag::Writer &streamOut, Importer::Model::Submesh const &submesh)
	{
	}

	void writeBone(BinaryTag::Writer &streamOut, Importer::Model::Bone const &bone)
	{
	}

	void buildSkeleton(
		std::unordered_set<aiNode*> const &boneset, 
		std::vector<Importer::Model::Bone> &bones, 
		unsigned int par,
		aiNode *cur)
	{
		if (boneset.find(cur)==boneset.end()) {
			return;
		}
		Importer::Model::Bone b;
		unsigned int boneId = bones.size();
		auto &tf=cur->mTransformation;
		b.transform=glm::mat4(
			tf.a1,tf.b1,tf.c1,tf.d1,
			tf.a2,tf.b2,tf.c2,tf.d2,
			tf.a3,tf.b3,tf.c3,tf.d3,
			tf.a4,tf.b4,tf.c4,tf.d4);
		b.parent=par;
		std::strncpy(b.name,cur->mName.C_Str(),32);
		bones.push_back(b);
		for (unsigned int ic=0; ic<cur->mNumChildren; ++ic) {
			buildSkeleton(boneset,bones,boneId,cur->mChildren[ic]);
		}
	}

	int findBone(std::vector<Importer::Model::Bone> bones, const char *name)
	{
		for (unsigned int ib = 0; ib<bones.size(); ++ib) {
			if (!strncmp(name, bones[ib].name, 32)) {
				return ib;
			}
		}
		return -1;
	}
}

// pack matrix
namespace msgpack {
	MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
		template <typename Stream>
		packer<Stream>& operator<<(packer<Stream>& o, glm::mat4 const& v) {
			o.pack_array(16);
			for (unsigned int i = 0; i < 4; ++i) {
				for (unsigned int j = 0; j < 4; ++j) {
					o.pack_float(v[i][j]);
				}
			}
			return o;
		}
	} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

namespace Importer {

Model::Model(const char *filePath)
{
	import(filePath);
}

void Model::import(const char *filePath)
{
	Assimp::Importer importer;
	const aiScene* scene=importer.ReadFile(
		filePath,
		aiProcess_CalcTangentSpace|
		aiProcess_Triangulate|
		aiProcess_JoinIdenticalVertices|
		aiProcess_SortByPType);
	if (!scene) {
		ERROR<<importer.GetErrorString();
		assert(false);
	}
	unsigned int nbvertex=0,nbindex=0,nbbones=0;
	for (unsigned int i=0; i<scene->mNumMeshes; ++i) {
		nbvertex+=scene->mMeshes[i]->mNumVertices;
		nbindex+=scene->mMeshes[i]->mNumFaces*3;
		nbbones+=scene->mMeshes[i]->mNumBones;
	}
	vertices.resize(nbvertex);
	indices.resize(nbindex);
	unsigned int boneid=0,vertexid=0,vertexbase=0,indexbase=0;
	// set of all nodes associated with a bone and their parents
	std::unordered_set<aiNode*> boneset;
	for (unsigned int i=0; i<scene->mNumMeshes; ++i) {
		auto mesh=scene->mMeshes[i];
		submeshes.push_back(
			Model::Submesh{
				vertexbase,
				indexbase,
				mesh->mNumVertices,
				mesh->mNumFaces*3});
		for (unsigned int iv=0; iv<mesh->mNumVertices; ++iv) {
			auto &v=mesh->mVertices[iv];
			auto &n=mesh->mNormals[iv];
			auto t=mesh->mTextureCoords[0];
			vertices[vertexid].position=glm::vec3(v.x,v.y,v.z);
			//vertices[vertexid].normal = glm::vec3(n.x, n.y, n.z);
			if (mesh->HasTextureCoords(0))
				vertices[vertexid].texcoord=glm::vec2(t[iv].x,t[iv].y);
			else
				vertices[vertexid].texcoord=glm::vec2(0,0);
			++vertexid;
		}
		for (unsigned int ii=0; ii<mesh->mNumFaces; ++ii) {
			auto &face=mesh->mFaces[ii];
			assert(face.mNumIndices==3);
			indices[indexbase+ii*3]=vertexbase+face.mIndices[0];
			indices[indexbase+ii*3+1]=vertexbase+face.mIndices[1];
			indices[indexbase+ii*3+2]=vertexbase+face.mIndices[2];
		}
		for (unsigned int ib=0; ib<mesh->mNumBones; ++ib) {
			// mark the nodes that are associated to a bone
			auto bone=mesh->mBones[ib];
			auto node=findNodeWithName(scene->mRootNode,bone->mName);
			addBranch(boneset,node);
		}
		vertexbase+=mesh->mNumVertices;
		indexbase+=mesh->mNumFaces*3;
	}

	// wow
	// such clusterfuck
	// very bullshit
	buildSkeleton(boneset,bones,-1,scene->mRootNode);
	// second pass: assign bone IDs and weights
	vertexbase=0;
	for (unsigned int i=0; i<scene->mNumMeshes; ++i) {
		auto mesh=scene->mMeshes[i];
		for (unsigned int ib=0; ib<mesh->mNumBones; ++ib) {
			auto bone=mesh->mBones[ib];
			auto boneid=findBone(bones, bone->mName.C_Str());
			auto &bone2=bones[boneid];
			auto &m=bone->mOffsetMatrix;
			bone2.invBindPose = glm::mat4(
				m.a1, m.b1, m.c1, m.d1,
				m.a2, m.b2, m.c2, m.d2,
				m.a3, m.b3, m.c3, m.d3,
				m.a4, m.b4, m.c4, m.d4);
			for (unsigned int iv=0; iv<bone->mNumWeights; ++iv) {
				auto w=bone->mWeights[iv];
				int iw;
				for (iw=0; iw<4; ++iw) {
					if (vertices[vertexbase+w.mVertexId].boneWeights[iw]==0.0f) {
						vertices[vertexbase+w.mVertexId].boneIds[iw]=boneid;
						vertices[vertexbase+w.mVertexId].boneWeights[iw]=w.mWeight;
						break;
					}
				}
				assert(iw!=4);
			}
		}
		vertexbase+=mesh->mNumVertices;
	}
}

void Model::export(std::ostream &streamOut)
{
	//BinaryTag::Writer w(streamOut);
	//w.beginCompound("mesh");
	//w.beginCompound("submeshes");
	//	for (auto &&submesh : submeshes) {
	//		w.beginCompound("");
	//		w.writeUint("startIndex", submesh.startIndex);
	//		w.writeUint("startVertex", submesh.startVertex);
	//		w.writeUint("numIndices", submesh.numIndices);
	//		w.writeUint("numVertices", submesh.numVertices);
	//		w.endCompound();
	//	}
	//w.endCompound();
	//w.beginCompound("bones");
	//	for (auto &&bone : bones) {
	//		w.beginCompound("");
	//		w.writeString("name", bone.name);
	//		w.writeFloatArray("invBindPose", &bone.invBindPose[0], &bone.invBindPose[0] + 16);
	//		w.writeFloatArray("transform", &bone.transform[0], &bone.transform[0] + 16);
	//		w.writeInt("parent", bone.parent);
	//		w.endCompound();
	//	}
	//w.endCompound();
	//w.beginCompound("vertices");
	//	w.writeUint("numVertices", vertices.size());
	//	w.writeUintArray("layout", {
	//		/*position   */(unsigned int)ElementFormat::Float3,
	//		/*normal     */(unsigned int)ElementFormat::Unorm10x3_1x2,
	//		/*tangent    */(unsigned int)ElementFormat::Unorm10x3_1x2,
	//		/*bitangent  */(unsigned int)ElementFormat::Unorm10x3_1x2,
	//		/*texcoord   */(unsigned int)ElementFormat::Unorm16x2,
	//		/*boneids    */(unsigned int)ElementFormat::Uint8x4,
	//		/*boneweights*/(unsigned int)ElementFormat::Unorm16x4
	//	});
	//	w.writeBlob("data", vertices.data(), sizeof(Importer::Model::Vertex) * vertices.size());
	//w.endCompound();
	//w.beginCompound("indices");
	//	w.writeUint("numIndices", indices.size());
	//	w.writeUint("type", (unsigned int)ElementFormat::Uint32);
	//	w.writeBlob("data", indices.data(), sizeof(uint32_t) * indices.size());
	//w.endCompound();
	//w.endCompound();
	std::cout << "Exporting...\n";
	auto packer = msgpack::packer<std::ostream>(streamOut);
	
	packer.pack_array(submeshes.size());
	for (auto &&submesh : submeshes) {
		msgpack::pack(streamOut, submesh.startIndex);
		msgpack::pack(streamOut, submesh.startVertex);
		msgpack::pack(streamOut, submesh.numIndices);
		msgpack::pack(streamOut, submesh.numVertices);
	}
	packer.pack_array(bones.size());
	for (auto &&bone : bones) {
		msgpack::pack(streamOut, bone.name);
		msgpack::pack(streamOut, bone.invBindPose);
		msgpack::pack(streamOut, bone.transform);
		msgpack::pack(streamOut, bone.parent);
	}
	packer.pack_uint32(vertices.size());
	packer.pack_uint32(indices.size());
	packer.pack_bin_body((char*)vertices.data(), vertices.size()*sizeof(Model::Vertex));
	packer.pack_bin_body((char*)indices.data(), indices.size()*sizeof(uint32_t));
}

}