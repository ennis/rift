#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <rendering/opengl4/graphics_context.hpp>
#include <rendering/opengl4/pass.hpp>
#include <rendering/opengl4/light.hpp>
#include <transform.hpp>

namespace gl4
{
	enum class MaterialType
	{
		Lambertian,
		Reflective,
		Glass
	};

	// A shader (collection of shader variants)
	struct Shader
	{
		using Ptr = std::unique_ptr<Shader>;

		MaterialType materialType = MaterialType::Lambertian;
		PipelineState::Ptr variant_ShadowStandard = nullptr;
		PipelineState::Ptr variant_Forward_PointLight = nullptr;
		PipelineState::Ptr variant_Forward_SpotLight = nullptr;
		PipelineState::Ptr variant_Forward_DirectionalLight = nullptr;
		PipelineState::Ptr variant_Deferred = nullptr;

		static Ptr loadFromFile(const char *path);
	};

	struct Material
	{
		using Ptr = std::unique_ptr<Material>;

		Material() = default;

		// Render an object under the given light
		void prepareForwardPass(
			GraphicsContext &gc,
			ForwardPassContext &passContext, 
			const Transform &modelToWorld
			);

		// TODO
		void prepareShadowPass(
			GraphicsContext &gc,
			ShadowPassContext &passContext,
			const Transform &modelToWorld
			);

		Shader *shader = nullptr;
		Texture2D *diffuseMap = nullptr;
		Texture2D *normalMap = nullptr;
		Buffer *userParams = nullptr;
	};

	Buffer *createSceneViewUBO(GraphicsContext &gc, const SceneView &sv);
}

#endif