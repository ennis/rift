#include <rendering/opengl4.hpp>
#include <utils/binary_io.hpp>
#include <log.hpp>

namespace 
{
	GLuint compileShaderVariant(
		const std::string &vs_source, 
		const std::string &ps_source, 
		util::array_ref<ShaderKeyword> keywords)
	{
		auto vs = compileShader(vs_source.c_str(), "", gl::VERTEX_SHADER, keywords);
		auto ps = compileShader(ps_source.c_str(), "", gl::FRAGMENT_SHADER, keywords);
		auto prog = compileProgram(vs, 0, ps);
		gl::DeleteShader(vs);
		gl::DeleteShader(ps);
		return prog;
	}
}

Shader *loadShaderAsset(AssetDatabase &assetDb, GraphicsContext &gc, std::string assetId)
{
	return assetDb.loadAsset<Shader>(assetId, [&]{
		auto src = loadShaderSource(assetId.c_str());
		auto ptr = std::make_unique<Shader>();
		ptr->programForwardPointLight = compileShaderVariant(src, src, { { "POINT_LIGHT" } });
		ptr->programForwardDirectionalLight = compileShaderVariant(src, src, { { "DIRECTIONAL_LIGHT" } });
		ptr->programForwardSpotLight = compileShaderVariant(src, src, { { "SPOT_LIGHT" } });
		return std::move(ptr);
	});
}