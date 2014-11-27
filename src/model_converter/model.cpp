#include "model.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>
#include <serialization.hpp>
#include <unordered_set>
#include <renderer.hpp>
#include <cassert>

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

	aiNode *findNodeWithMesh(aiNode *root, unsigned int iMesh)
	{
		for (unsigned int im = 0; im < root->mNumMeshes; ++im) {
			if (root->mMeshes[im] == iMesh) {
				return root;
			}
		}
		for (unsigned int ic = 0; ic<root->mNumChildren; ++ic) {
			auto f = findNodeWithMesh(root->mChildren[ic], iMesh);
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

	void buildSkeleton(
		std::unordered_set<aiNode*> const &boneset, 
		std::vector<Importer::Model::Bone> &bones, 
		std::vector<Importer::Model::Submesh> &submeshes,
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
		// does the node has associated meshes?
		for (unsigned int im = 0; im < cur->mNumMeshes; ++im) {
			auto meshindex = cur->mMeshes[im];
			submeshes[meshindex].bone = bones.size()-1;
			LOG << cur->mName.C_Str() << "-> submesh " << meshindex;
		}
		for (unsigned int ic=0; ic<cur->mNumChildren; ++ic) {
			buildSkeleton(boneset,bones,submeshes,boneId,cur->mChildren[ic]);
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


namespace rift {
namespace serialization {
	// glm::mat4 packer
	template <>
	struct pack_traits < glm::mat4 > {
		static void pack(Packer &p, glm::mat4 const &v) {
			for (unsigned int i = 0; i < 4; ++i) {
				for (unsigned int j = 0; j < 4; ++j) {
					p.pack(v[i][j]);
				}
			}
		}
	};

	template <>
	struct pack_traits < glm::vec2 > {
		static void pack(Packer &p, glm::vec2 const &v) {
			p.pack(v[0]);
			p.pack(v[1]);
		}
	};

	template <>
	struct pack_traits < glm::vec3 > {
		static void pack(Packer &p, glm::vec3 const &v) {
			p.pack(v[0]);
			p.pack(v[1]);
			p.pack(v[2]);
		}
	};

	template <>
	struct pack_traits < glm::vec4 > {
		static void pack(Packer &p, glm::vec4 const &v) {
			p.pack(v[0]);
			p.pack(v[1]);
			p.pack(v[2]);
			p.pack(v[3]);
		}
	}; 
	
	template <>
	struct pack_traits < glm::u8vec4 > {
		static void pack(Packer &p, glm::u8vec4 const &v) {
			char ch[4] = { v[0], v[1], v[2], v[3] };
			p.pack_n(ch, ch+4);
		}
	};

}}

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
	positions.reserve(nbvertex);
	normals.reserve(nbvertex);
	tangents.reserve(nbvertex);
	bitangents.reserve(nbvertex);
	texcoords.reserve(nbvertex);
	indices.reserve(nbindex);

	unsigned int boneid = 0, vertexid = 0, vertexbase = 0, indexbase = 0;
	isSkinned = false;
	// set of all nodes associated with a bone/submesh and their parents
	std::unordered_set<aiNode*> boneset;
	for (unsigned int i=0; i<scene->mNumMeshes; ++i) {
		auto mesh=scene->mMeshes[i];
		auto node = findNodeWithMesh(scene->mRootNode, i);
		if (node) addBranch(boneset, node);
		submeshes.push_back(
			Model::Submesh{
				vertexbase,
				indexbase,
				mesh->mNumVertices,
				mesh->mNumFaces*3,
				-1});
		LOG << "submesh SV:"
			<< vertexbase
			<< " SI:" << indexbase
			<< " NV:" << mesh->mNumVertices
			<< " NI:" << mesh->mNumFaces * 3;
		// mark the nodes that are associated with the submesh
		//addBranch(boneset, mesh->)
		for (unsigned int iv=0; iv<mesh->mNumVertices; ++iv) {
			auto &v=mesh->mVertices[iv];
			auto &n=mesh->mNormals[iv];
			auto t = mesh->mTextureCoords[0];
			auto &tg = mesh->mTangents[iv];
			auto &btg = mesh->mBitangents[iv];
			positions.push_back(glm::vec3(v.x,v.y,v.z));
			normals.push_back(glm::vec3(n.x,n.y,n.z));
			if (mesh->HasTextureCoords(0)) {
				texcoords.push_back(glm::vec2(t[iv].x, t[iv].y));
				tangents.push_back(glm::vec3(tg.x, tg.y, tg.z));
				bitangents.push_back(glm::vec3(btg.x, btg.y, btg.z));
			}
			else {
				LOG << "Mesh " << i << " has no texture coordinates";
				texcoords.emplace_back(glm::vec2(0, 0));
				tangents.emplace_back(0, 0, 0);
				bitangents.emplace_back(0, 0, 0);
			}
			++vertexid;
		}
		for (unsigned int ii=0; ii<mesh->mNumFaces; ++ii) {
			auto &face=mesh->mFaces[ii];
			assert(face.mNumIndices==3);
			indices.emplace_back(face.mIndices[0]);
			indices.emplace_back(face.mIndices[1]);
			indices.emplace_back(face.mIndices[2]);
		}
		for (unsigned int ib=0; ib<mesh->mNumBones; ++ib) {
			// mark the nodes that are associated to a bone
			isSkinned = true;
			auto bone=mesh->mBones[ib];
			auto node = findNodeWithName(scene->mRootNode, bone->mName);
			LOG << "findNodeWithName " << bone->mName.C_Str() << "->" << node;
			addBranch(boneset,node);
		}
		vertexbase+=mesh->mNumVertices;
		indexbase+=mesh->mNumFaces*3;
	}

	// wow
	// such clusterfuck
	// very bullshit
	buildSkeleton(boneset,bones,submeshes,-1,scene->mRootNode);
	// debug: show bones
	for (auto const &b : boneset) {
		LOG << b->mName.C_Str();
	}

	if (isSkinned) {
		// second pass: assign bone IDs and weights
		boneIds.resize(nbvertex);
		boneWeights.resize(nbvertex);
		vertexbase = 0;
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			auto mesh = scene->mMeshes[i];
			for (unsigned int ib = 0; ib < mesh->mNumBones; ++ib) {
				auto bone = mesh->mBones[ib];
				auto boneid = findBone(bones, bone->mName.C_Str());
				auto &bone2 = bones[boneid];
				auto &m = bone->mOffsetMatrix;
				bone2.invBindPose = glm::mat4(
					m.a1, m.b1, m.c1, m.d1,
					m.a2, m.b2, m.c2, m.d2,
					m.a3, m.b3, m.c3, m.d3,
					m.a4, m.b4, m.c4, m.d4);
				for (unsigned int iv = 0; iv < bone->mNumWeights; ++iv) {
					auto w = bone->mWeights[iv];
					int iw;
					for (iw = 0; iw < 4; ++iw) {
						if (boneWeights[vertexbase + w.mVertexId][iw] == 0.0f) {
							boneIds[vertexbase + w.mVertexId][iw] = boneid;
							boneWeights[vertexbase + w.mVertexId][iw] = w.mWeight;
							break;
						}
					}
					assert(iw != 4);
				}
			}
			vertexbase += mesh->mNumVertices;
		}
	}
}

void Model::exportModel(std::ostream &streamOut)
{
	using namespace rift::serialization;
	std::cout << "Exporting...\n";
	unsigned int nv = positions.size(), ni = indices.size();
	assert(texcoords.size() == nv && normals.size() == nv && bitangents.size() == nv && tangents.size() == nv);
	Packer packer(streamOut);
	packer.pack(submeshes, [](Packer &p, const Submesh &sm) {
		p.pack(sm.startVertex);
		p.pack(sm.startIndex);
		p.pack(sm.numVertices);
		p.pack(sm.numIndices);
		p.pack(sm.bone);
	});
	packer.pack(bones, [](Packer &p, const Bone &b) {
		p.pack(b.name);
		p.pack(b.transform);
		p.pack(b.invBindPose);
		p.pack(b.parent);
	});
	packer.pack(nv);
	packer.pack(ni);
	packer.pack(isSkinned ? 1 : 0);	// format
	packer.pack_n(positions.cbegin(), positions.cend());
	packer.pack_n(normals.cbegin(), normals.cend());
	packer.pack_n(tangents.cbegin(), tangents.cend());
	packer.pack_n(bitangents.cbegin(), bitangents.cend());
	packer.pack_n(texcoords.cbegin(), texcoords.cend());
	if (isSkinned) {
		packer.pack_n(boneIds.cbegin(), boneIds.cend());
		packer.pack_n(boneWeights.cbegin(), boneWeights.cend());
	}
	for (auto index : indices) {
		packer.pack16((unsigned short)index);
	}
}

}
