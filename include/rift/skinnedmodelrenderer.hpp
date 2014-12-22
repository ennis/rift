#ifndef SKINNEDMODELRENDERER_HPP
#define SKINNEDMODELRENDERER_HPP

#include <model.hpp>
#include <renderable.hpp>
#include <transform.hpp>
#include <vector>
#include <pose.hpp>

class SkinnedModelRenderer
{
public:
	SkinnedModelRenderer() = default;
	SkinnedModelRenderer(Renderer &renderer, Model &model);
	// non-copyable
	SkinnedModelRenderer(const SkinnedModelRenderer &) = delete;
	SkinnedModelRenderer &operator=(const SkinnedModelRenderer &rhs) = delete;
	// move-assignable
	SkinnedModelRenderer(SkinnedModelRenderer &&rhs);
	SkinnedModelRenderer &operator=(SkinnedModelRenderer &&rhs);
	~SkinnedModelRenderer();
	
	void applyPose(const Pose &pose);
	void draw(const RenderContext &context);

private:
	Renderer *mRenderer;
	Model *mModel;
	// final vertices after skinning
	std::vector<glm::vec3> mFinalVertices;
	// final bone transforms
	std::vector<glm::mat4> mFinalTransforms;
	// final mesh buffer
	Mesh mMesh;
	Shader *mShader;
};

 
#endif /* end of include guard: SKINNEDMODELRENDERER_HPP */