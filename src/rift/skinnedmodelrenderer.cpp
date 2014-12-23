#include <skinnedmodelrenderer.hpp>
#include <immediatecontext.hpp>
#include <pose.hpp>
#include <effect.hpp>

namespace {
	void calculateTransformsRec(
		const std::vector<Model::Bone> &bones,
		const Pose &pose,
		std::vector<glm::mat4> &transforms,
		glm::mat4 currentTransform,
		int boneIndex)
	{
		auto bone = bones[boneIndex];
		Transform t;
		t.rotation = pose.getRotations()[boneIndex];
		t.position = pose.getPositions()[boneIndex];
		transforms[boneIndex] = currentTransform * t.toMatrix();
		for (auto childBoneIndex : bone.children) {
			auto child_tf = currentTransform * bones[childBoneIndex].transform;
			calculateTransformsRec(bones, pose, transforms, child_tf, childBoneIndex);
		}
	}

	void drawSkeletonRec(
		ImmediateContext &context,
		std::vector<Model::Bone> const &bones,
		glm::mat4 transform,
		int boneIndex)
	{
		auto bone = bones[boneIndex];
		for (auto childBoneIndex : bone.children) {
			auto child_tf = transform * bones[childBoneIndex].transform;
			context.addVertex(::Vertex(glm::vec3(transform*glm::vec4(0, 0, 0, 1)), glm::vec4(0, 1, 0, 1)));
			context.addVertex(::Vertex(glm::vec3(child_tf * glm::vec4(0, 0, 0, 1)), glm::vec4(0, 1, 0, 1)));
			drawSkeletonRec(context, bones, child_tf, childBoneIndex);
		}
	}

	void skinning(
		std::vector<glm::vec3> const &positions,
		std::vector<Model::Bone> const &bones,
		std::vector<glm::u8vec4> const &boneIndices,
		std::vector<glm::vec4> const &boneWeights,
		std::vector<glm::vec3> &outPositions,
		std::vector<glm::mat4> const &transforms)
	{
		auto nv = positions.size();
		outPositions.resize(nv);
		for (unsigned int iv = 0; iv<nv; ++iv) {
			auto const &v = positions[iv];
			auto &vout = outPositions[iv];
			auto indices = boneIndices[iv];
			auto weights = boneWeights[iv];
			if (indices[0] == -1) continue;	// ???
			// bwaaaaah
			auto tf =
				transforms[indices[0]] * weights[0] +
				transforms[indices[1]] * weights[1] +
				transforms[indices[2]] * weights[2] +
				transforms[indices[3]] * weights[3];
			vout = glm::vec3(tf * glm::vec4(v, 1.0f));
		}
	}
}

SkinnedModelRenderer::SkinnedModelRenderer(Renderer &renderer, Model &model) :
mRenderer(&renderer),
mModel(&model),
mMesh()
{
	auto nv = model.getNumVertices();
	auto nbones = model.getNumIndices();
	mFinalVertices.resize(nv);
	mFinalTransforms.resize(nbones);
	Mesh::Attribute attribs[] = { {0, ElementFormat::Float3} };
	Mesh::Buffer buffers[] = { { ResourceUsage::Dynamic } };
	mMesh.allocate(renderer, PrimitiveType::Triangle, 1, attribs, 1, buffers, nv, nullptr, model.getNumIndices(), ElementFormat::Uint16, ResourceUsage::Static, mModel->getIndices().data());
	mShader = renderer.createShader(
		loadShaderSource("resources/shaders/model/vert.glsl").c_str(),
		loadShaderSource("resources/shaders/model/frag.glsl").c_str());
}

SkinnedModelRenderer::SkinnedModelRenderer(SkinnedModelRenderer &&rhs)
{
	*this = std::move(rhs);
}

SkinnedModelRenderer &SkinnedModelRenderer::operator=(SkinnedModelRenderer &&rhs)
{
	mRenderer = std::move(rhs.mRenderer);
	mModel = std::move(rhs.mModel);
	mFinalVertices = std::move(rhs.mFinalVertices);
	mFinalTransforms = std::move(rhs.mFinalTransforms);
	mMesh = std::move(rhs.mMesh);
	mShader = std::move(rhs.mShader);
	return *this;
}

SkinnedModelRenderer::~SkinnedModelRenderer()
{}

void SkinnedModelRenderer::applyPose(const Pose &pose)
{
	auto const &bones = mModel->getBones();
	assert(pose.getNumBones() == bones.size());
	calculateTransformsRec(bones, pose, mFinalTransforms, glm::mat4(1.0f), 0);
}

void SkinnedModelRenderer::draw(const RenderContext &context)
{
	//debugDraw->clear(); 
	//drawSkeletonRec(*debugDraw, mModel->getBones(), glm::mat4(1.0f), 0);
	//debugDraw->render(context);
	// render with skinning
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (mModel->isSkinned())
	{
		skinning(
			mModel->getPositions(),
			mModel->getBones(),
			mModel->getBoneIndices(),
			mModel->getBoneWeights(),
			mFinalVertices,
			mFinalTransforms);
		mMesh.update(0, 0, mModel->getNumVertices(), mFinalVertices.data());
		mRenderer->setShader(mShader);
		mRenderer->setConstantBuffer(0, context.perFrameShaderParameters);
		mRenderer->setNamedConstantMatrix4("modelMatrix", glm::mat4(1.0f));
		for (auto &&sm : mModel->getSubmeshes()) {
			mMesh.drawPart(sm.startVertex, sm.startIndex, sm.numIndices);
		}
	}
	else {
		// no skinning
		mMesh.update(0, 0, mModel->getNumVertices(), mModel->getPositions().data());
		mRenderer->setShader(mShader);
		mRenderer->setConstantBuffer(0, context.perFrameShaderParameters);
		auto const &submeshes = mModel->getSubmeshes();
		for (unsigned int i = 0; i < submeshes.size(); ++i) {
			auto const &sm = submeshes[i];
			mRenderer->setNamedConstantMatrix4("modelMatrix", mFinalTransforms[sm.bone]);
			mMesh.drawPart(sm.startVertex, sm.startIndex, sm.numIndices);
		}
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}