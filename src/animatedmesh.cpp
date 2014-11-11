#include <animatedmesh.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>

namespace {
	aiNode *findNodeWithName(aiNode *root, aiString &name)
	{
		if (root->mName == name) {
			return root;
		}
		for (unsigned int ic = 0; ic < root->mNumChildren; ++ic) {
			auto f = findNodeWithName(root->mChildren[ic], name);
			if (f) return f;
		}
		return nullptr;
	}

	void addBranch(std::unordered_set<aiNode*> &map, aiNode *node)
	{
		if (node != nullptr) {
			map.insert(node);
			addBranch(map, node->mParent);
		}
	}

}

void AnimatedMesh::loadFromFile(const char *filePath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		filePath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);
	if (!scene) {
		ERROR << importer.GetErrorString();
		assert(false);
	}
	unsigned int nbvertex = 0, nbindex = 0, nbbones = 0;
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		nbvertex += scene->mMeshes[i]->mNumVertices;
		nbindex += scene->mMeshes[i]->mNumFaces * 3;
		nbbones += scene->mMeshes[i]->mNumBones;
	}
	assert(nbbones > 0 && nbbones < kMaxBones);
	ElementFormat layoutdesc[] = {
		ElementFormat::Float3,
		ElementFormat::Float3,
		ElementFormat::Float2,
		ElementFormat::Uint8x4,
		ElementFormat::Float4 };
	mMesh.allocate(
		PrimitiveType::Triangle, 5, layoutdesc, ResourceUsage::Static, nbvertex, nullptr, nbindex, nullptr);
	Vertex *vtx = new Vertex[nbvertex];
	uint16_t *idx = new uint16_t[nbindex];
	unsigned int boneid = 0, vertexid = 0, vertexbase=0, indexbase=0;
	// set of all nodes associated with a bone and their parents
	std::unordered_set<aiNode*> boneset;
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		auto mesh = scene->mMeshes[i];
		// flatten mesh hierarchy
		// TODO submeshes
		for (unsigned int iv = 0; iv < mesh->mNumVertices; ++iv) {
			auto &v = mesh->mVertices[iv];
			auto &n = mesh->mNormals[iv];
			auto t = mesh->mTextureCoords[0];
			vtx[vertexid].position = glm::vec3(v.x, v.y, v.z);
			//vtx[vertexid].normal = glm::vec3(n.x, n.y, n.z);
			if (mesh->HasTextureCoords(0))
				vtx[vertexid].texcoord = glm::vec2(t[iv].x, t[iv].y);
			else
				vtx[vertexid].texcoord = glm::vec2(0, 0);
			++vertexid;
		}
		for (unsigned int ii = 0; ii < mesh->mNumFaces; ++ii) {
			auto &face = mesh->mFaces[ii];
			assert(face.mNumIndices == 3);
			idx[indexbase + ii * 3] = vertexbase + face.mIndices[0];
			idx[indexbase + ii * 3 + 1] = vertexbase + face.mIndices[1];
			idx[indexbase + ii * 3 + 2] = vertexbase + face.mIndices[2];
		}
		for (unsigned int ib = 0; ib < mesh->mNumBones; ++ib) {
			// prune the nodes that are associated to a bone
			auto bone = mesh->mBones[ib];
			auto node = findNodeWithName(scene->mRootNode, bone->mName);
			addBranch(boneset, node);
		}
		vertexbase += mesh->mNumVertices;
		indexbase += mesh->mNumFaces * 3;
	}
	// wow
	// such clusterfuck
	// very bullshit
	buildSkeleton(boneset, nullptr, scene->mRootNode);
	// second pass: assign bone IDs and weights
	vertexbase = 0;
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		auto mesh = scene->mMeshes[i];
		for (unsigned int ib = 0; ib < mesh->mNumBones; ++ib) {
			assert(boneid < kMaxBones);
			auto bone = mesh->mBones[ib];
			auto boneid = findBone(bone->mName.C_Str());
			auto &bone2 = mBones[boneid];
			auto &m = bone->mOffsetMatrix;
			bone2.invBindPose = glm::transpose(glm::mat4(
				m.a1, m.a2, m.a3, m.a4,
				m.b1, m.b2, m.b3, m.b4,
				m.c1, m.c2, m.c3, m.c4,
				m.d1, m.d2, m.d3, m.d4));
			for (unsigned int iv = 0; iv < bone->mNumWeights; ++iv) {
				auto w = bone->mWeights[iv];
				int iw;
				for (iw = 0; iw < 4; ++iw) {
					if (vtx[vertexbase + w.mVertexId].boneWeights[iw] == 0.0f) {
						vtx[vertexbase + w.mVertexId].boneIds[iw] = boneid;
						vtx[vertexbase + w.mVertexId].boneWeights[iw] = w.mWeight;
						break;
					}
				}
				assert(iw != 4);
			}
		}
		vertexbase += mesh->mNumVertices;
	}

	ElementFormat layout[] = {
		ElementFormat::Float3,
		ElementFormat::Float3, 
		ElementFormat::Float2, 
		ElementFormat::Uint8x4, 
		ElementFormat::Float4
	};
	mMesh.allocate(PrimitiveType::Triangle, 5, layout, ResourceUsage::Dynamic, nbvertex, vtx, nbindex, idx);
	mNumVertices = nbvertex;
	mVertices = vtx;

	// shader
	mShader = mRenderer->createShader(
		loadShaderSource("resources/shaders/immediate/vert.glsl").c_str(),
		loadShaderSource("resources/shaders/immediate/frag.glsl").c_str());

	mVerticesSkinned = new Vertex[mNumVertices];

	delete[] idx;
}

unsigned int AnimatedMesh::findBone(const char *name)
{
	for (unsigned int ib = 0; ib < mNumBones; ++ib) {
		if (!strncmp(name, mBones[ib].name, 32)) {
			return ib;
		}
	}
	return -1;
}

void AnimatedMesh::buildSkeleton(std::unordered_set<aiNode*> &map, Bone *parent, aiNode *current)
{
	if (map.find(current) == map.end()) {
		return;
	}
	auto &b = mBones[mNumBones++];
	auto &tf = current->mTransformation;
	b.transform = glm::transpose(glm::mat4(
		tf.a1, tf.a2, tf.a3, tf.a4,
		tf.b1, tf.b2, tf.b3, tf.b4,
		tf.c1, tf.c2, tf.c3, tf.c4,
		tf.d1, tf.d2, tf.d3, tf.d4));
	b.parent = parent;
	if (parent) {
		parent->children[parent->numChildren++] = &b;
	}
	std::strncpy(b.name, current->mName.C_Str(), 32);
	for (unsigned int ic = 0; ic < current->mNumChildren; ++ic) {
		buildSkeleton(map, &b, current->mChildren[ic]);
	}
}


void AnimatedMesh::render(RenderContext const &renderContext)
{
	// skeleton
	immediateContext->clear();
	renderBone(mBones[0], glm::mat4(1.0f));
	immediateContext->render(renderContext);
	// skinning
	for (unsigned int iv = 0; iv < mNumVertices; ++iv) {
		auto const &v = mVertices[iv];
		auto &vout = mVerticesSkinned[iv];
		if (v.boneIds[0] == -1) continue;
		// bwaaaaah
		auto tf = 
			mBones[v.boneIds[0]].finalTransform * mBones[v.boneIds[0]].invBindPose * v.boneWeights[0] +
			mBones[v.boneIds[1]].finalTransform * mBones[v.boneIds[0]].invBindPose * v.boneWeights[1] +
			mBones[v.boneIds[2]].finalTransform * mBones[v.boneIds[0]].invBindPose * v.boneWeights[2] +
			mBones[v.boneIds[3]].finalTransform * mBones[v.boneIds[0]].invBindPose * v.boneWeights[3];
		vout.position = glm::vec3(tf * glm::vec4(v.position, 1.0f));
	}
	mMesh.update(0, mNumVertices, mVerticesSkinned);
	// render mesh
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	mRenderer->setShader(mShader);
	mRenderer->setConstantBuffer(0, renderContext.perFrameShaderParameters);
	mMesh.draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void AnimatedMesh::renderBone(Bone &bone, glm::mat4 const &transform)
{
	//LOG << id;
	bone.finalTransform = transform;
	for (unsigned int ic=0; ic<bone.numChildren; ++ic) {
		auto &c = *bone.children[ic];
		auto child_tf = transform * c.transform;
		auto tfv = child_tf * glm::vec4(0, 0, 0, 1);
		immediateContext->addVertex(::Vertex(glm::vec3(transform * glm::vec4(0, 0, 0, 1)), glm::vec4(0, 1, 0, 1)));
		immediateContext->addVertex(::Vertex(glm::vec3(child_tf * glm::vec4(0, 0, 0, 1)), glm::vec4(0, 1, 0, 1)));
		renderBone(c, child_tf);
	}
}