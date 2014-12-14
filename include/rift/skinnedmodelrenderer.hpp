#ifndef SKINNEDMODELRENDERER_HPP
#define SKINNEDMODELRENDERER_HPP

#include <model.hpp>
#include <transform.hpp>
#include <vector>
#include <immediatecontext.hpp>
#include <pose.hpp>

class SkinnedModelRenderer
{
public:
	SkinnedModelRenderer(Renderer &renderer, ImmediateContextFactory &icf, Model &model);
	
	void applyPose(const Pose &pose);
	void draw(RenderContext const &context);

private:
	ImmediateContext *debugDraw;
	// final vertices after skinning
	std::vector<glm::vec3> mFinalVertices;
	// final bone transforms
	std::vector<glm::mat4> mFinalTransforms;
	Renderer &mRenderer;
	Model &mModel;
	// final mesh buffer
	Mesh mMesh;
	Shader *mShader;
};

 
#endif /* end of include guard: SKINNEDMODELRENDERER_HPP */