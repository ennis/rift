#ifndef ANIMATEDMESH_HPP
#define ANIMATEDMESH_HPP

#include <renderer.hpp>
#include <immediatecontext.hpp>
#include <meshbuffer.hpp>
#include <unordered_set>

class AnimatedMesh
{
public:
	static const unsigned int kMaxBones = 256;
	AnimatedMesh(ImmediateContextFactory &immediateContextFactory, Renderer &renderer) : mRenderer(&renderer), mMesh(renderer), mNumBones(0)
	{
		immediateContext = immediateContextFactory.create(kMaxBones*8, PrimitiveType::Line);
	}
	void loadFromFile(const char *filePath);
	void render(RenderContext const &renderContext);

private:
	Renderer *mRenderer;
	ImmediateContext *immediateContext;
	MeshBuffer mMesh;
	Shader *mShader;
	ConstantBuffer *mBonesCB;
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texcoord;
		glm::i8vec4 boneIds;
		glm::vec4 boneWeights;
	};
	Vertex *mVertices;
	Vertex *mVerticesSkinned;
	unsigned int mNumVertices;
	struct Bone {
		Bone() = default;
		char name[32];
		glm::mat4 invBindPose;
		glm::mat4 transform;
		glm::mat4 finalTransform;
		unsigned int numChildren = 0;
		Bone *parent = nullptr;
		Bone *children[32];
	};
	Bone mBones[kMaxBones];
	unsigned int mNumBones;
	void buildSkeleton(std::unordered_set<aiNode*> &map, Bone *parent, aiNode *current);
	void renderBone(Bone &bone, glm::mat4 const &transform);
	unsigned int findBone(const char *name);
};

#endif /* end of include guard: ANIMATEDMESH_HPP */