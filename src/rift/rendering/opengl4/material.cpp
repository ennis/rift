#include <rendering/opengl4/material.hpp>
#include <rendering/opengl4/shader_compiler.hpp>
#include <log.hpp>

namespace gl4
{
	struct PerObject
	{
		glm::mat4 modelToWorld;
	};

	void Material::prepareForwardPass(
		GraphicsContext &gc,
		ForwardPassContext &passContext,
		const Transform &modelToWorld)
	{
		auto &cmd = *passContext.cmdBuf;

		if (passContext.light->mode == LightMode::Directional)
			cmd.setPipelineState(shader->variant_Forward_DirectionalLight.get());
		else if (passContext.light->mode == LightMode::Point)
			cmd.setPipelineState(shader->variant_Forward_PointLight.get());
		else if (passContext.light->mode == LightMode::Spot)
			cmd.setPipelineState(shader->variant_Forward_SpotLight.get());
		else
			assert(!"Unsupported light mode");

		if (normalMap)
		{
			passContext.cmdBuf->setTextures(
			{ diffuseMap, normalMap },
			{ gc.getSampler_LinearClamp(), gc.getSampler_LinearClamp() });
		}
		else
		{
			passContext.cmdBuf->setTextures(
			{ diffuseMap },
			{ gc.getSampler_LinearClamp() });
		}

		auto perObjBuf = gc.allocTransientBuffer<PerObject>();
		perObjBuf.map()->modelToWorld = modelToWorld.toMatrix();

		if (!userParams)
			cmd.setConstantBuffers({ passContext.sceneViewUBO, passContext.lightParamsUBO, perObjBuf.buf });
		else
			cmd.setConstantBuffers({ passContext.sceneViewUBO, passContext.lightParamsUBO, perObjBuf.buf, userParams });

	}

	Buffer *createSceneViewUBO(GraphicsContext &gc, const SceneView &sv)
	{
		auto tbuf = gc.allocTransientBuffer<SceneView>();
		*(tbuf.map()) = sv;
		return tbuf.buf;
	}

	PipelineState::Ptr compileShaderVariant(
		const std::string &vs_source, 
		const std::string &ps_source, 
		util::array_ref<Keyword> keywords)
	{
		auto vs = gl4::compileShader(vs_source.c_str(), "", ShaderStage::VertexShader, keywords);
		auto ps = gl4::compileShader(ps_source.c_str(), "", ShaderStage::PixelShader, keywords);
		return gl4::PipelineState::create(
			vs.get(), 
			nullptr, 
			ps.get(), 
			RasterizerDesc{}, 
			DepthStencilDesc{}, 
			BlendStateRenderTargetDesc{});
	}
	
	Shader::Ptr Shader::loadFromFile(const char *path)
	{
		auto src = gl4::loadShaderSource(path);
		auto ptr = std::make_unique<Shader>();
		// load variants
		ptr->variant_Forward_PointLight = compileShaderVariant(src, src, {{"POINT_LIGHT"}});
		ptr->variant_Forward_DirectionalLight = compileShaderVariant(src, src, { { "DIRECTIONAL_LIGHT" } });
		ptr->variant_Forward_SpotLight = compileShaderVariant(src, src, { { "SPOT_LIGHT" } });
		return std::move(ptr);
	}
}