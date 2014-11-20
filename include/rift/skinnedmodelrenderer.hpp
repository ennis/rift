#ifndef SKINNEDMODELRENDERER_HPP
#define SKINNEDMODELRENDERER_HPP

#include <model.hpp>
#include <vector>

class SkinnedMeshRenderer
{
public:
	SkinnedMeshRenderer(Renderer &renderer, Model &model) : 
	mRenderer(renderer),
	mModel(model)
	{}
	
	void applyPose(std::vector<Transform> &pose);
	void draw();

private:
	// final vertices after skinning
	std::vector<glm::vec3> mFinalVertices;
	Renderer &mRenderer;
	Model &mModel;
};

 
#endif /* end of include guard: SKINNEDMODELRENDERER_HPP */