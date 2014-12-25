#ifndef MODELRENDERER_HPP
#define MODELRENDERER_HPP

#include <model.hpp>
#include <material.hpp>
#include <renderer.hpp>
#include <renderable.hpp>
#include <transform.hpp>
#include <vector>

class ModelRenderer
{
public:
	ModelRenderer() = default;
	ModelRenderer(const Model &model) : mModel(&model)
	{}

	void setMaterial(unsigned int submesh, const Material &material) 
	{
		assert(submesh < mMaterialMap.size());
		mMaterialMap[submesh] = &material;
	}

	void setModel(const Model &model)
	{
		mModel = &model;
		mMaterialMap.resize(0);
		mMaterialMap.resize(model.getSubmeshes().size(), nullptr);
	}

	const Model *getModel() const
	{
		return mModel;
	}

	void render(const RenderContext &context, const Transform &modelTransform);

private:
	const Model *mModel = nullptr;
	std::vector<const Material*> mMaterialMap;
};

 
#endif /* end of include guard: MODELRENDERER_HPP */