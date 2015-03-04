#ifndef EFFECT_HPP
#define EFFECT_HPP

#include <gl4/renderer.hpp>
#include <boost/filesystem.hpp>

namespace gl4
{
	class Effect
	{
	public:
		using Ptr = std::unique_ptr < Effect > ;

		struct Keyword
		{
			std::string define;
			std::string value;
		};

		// do not use
		Effect() = default;

		~Effect()
		{//TODO
		}

		// Compile a shader with the given list of keywords
		// TODO once the new effect system lands, there will be no need for preprocessing:
		// All the possible permutations will be created before runtime
		// TODO should return non-owning pointer?
		Shader::Ptr compileShader(
			std::array_ref<Keyword> additionalKeywords = {}
			); 
		
		// override render states
		Shader::Ptr compileShader(
			std::array_ref<Keyword> additionalKeywords,
			const RasterizerDesc &rasterizerState,
			const DepthStencilDesc &depthStencilState
			);

		static std::unique_ptr<Effect> loadFromFile(
			const char *combinedSourcePath,
			std::array_ref<Effect::Keyword> keywords = {}
		);

	private:
		boost::filesystem::path vs_path;
		boost::filesystem::path ps_path;
		std::string vs_source;
		std::string ps_source;
		// true if mVertexShader contains both the vertex shader and the fragment shader
		bool combined_source;
		// list of #defines
		std::vector<Keyword> keywords;
	};
}

 
#endif /* end of include guard: EFFECT_HPP */