#include <skinnedmodelrenderer.hpp>
#include <immediatecontext.hpp>

namespace {
	void calculateTransformsRec(
		std::vector<Model::Bone> const &bones,
		std::vector<glm::mat4> &transforms,
		glm::mat4 currentTransform,
		int boneIndex)
	{
		auto bone = bones[boneIndex];
		transforms[boneIndex] = currentTransform * bone.invBindPose;
		for (auto childBoneIndex : bone.children) {
			auto child_tf = currentTransform * bones[childBoneIndex].transform;
			calculateTransformsRec(bones, transforms, child_tf, childBoneIndex);
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

SkinnedModelRenderer::SkinnedModelRenderer(Renderer &renderer, ImmediateContextFactory &icf, Model &model) :
mRenderer(renderer),
mModel(model),
mMesh(renderer)
{
	auto nv = model.getPositions().size();
	auto nbones = model.getBones().size();
	mFinalVertices.resize(nv);
	mFinalTransforms.resize(nbones);
	debugDraw = icf.create(nbones * 2, PrimitiveType::Line);
	Mesh::Attribute attribs[] = { {0, ElementFormat::Float3} };
	Mesh::Buffer buffers[] = { { ResourceUsage::Dynamic } };
	mMesh.allocate(PrimitiveType::Triangle, 1, attribs, 1, buffers, nv, nullptr, model.numIndices(), ElementFormat::Uint16, ResourceUsage::Static, mModel.getIndices().data());
	mShader = renderer.createShader(
		loadShaderSource("resources/shaders/immediate/vert.glsl").c_str(),
		loadShaderSource("resources/shaders/immediate/frag.glsl").c_str());
}

void SkinnedModelRenderer::applyPose(std::vector<Transform> &pose)
{
	auto const &bones = mModel.getBones();
	calculateTransformsRec(bones, mFinalTransforms, glm::mat4(1.0f), 0);
}

void SkinnedModelRenderer::draw(RenderContext const &context)
{
	debugDraw->clear(); 
	drawSkeletonRec(*debugDraw, mModel.getBones(), glm::mat4(1.0), 0);
	debugDraw->render(context);
	skinning(
		mModel.getPositions(), 
		mModel.getBones(), 
		mModel.getBoneIndices(), 
		mModel.getBoneWeights(), 
		mFinalVertices, 
		mFinalTransforms);
	mMesh.update(0, 0, mModel.getPositions().size(), mFinalVertices.data());
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	mRenderer.setShader(mShader);
	mRenderer.setConstantBuffer(0, context.perFrameShaderParameters);
	for (auto &&sm : mModel.getSubmeshes()) {
		mMesh.drawPart(sm.startVertex, sm.startIndex, sm.numIndices);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}