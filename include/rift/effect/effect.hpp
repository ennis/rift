#ifndef EFFECT_HPP
#define EFFECT_HPP

#include <renderer.hpp>
#include <unordered_map>
#include <array>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <boost/filesystem/path.hpp>

enum class EffectParameterType
{
	Float,
	Float2,
	Float3, 
	Float4,
	Int,
	Int2,
	Int3, 
	Int4,
	Float3x4,
	Float4x4
};

class CompiledShader; 

std::string loadShaderSource(const char *path);

//=============================================================================
//
// classe Effect
//
// C'est une petite couche d'abstraction sur les shaders. Elle gère la création
// des shaders, fait du preprocessing (gère les includes et les #defines)
//
// Un Effect peut compiler plusieurs shaders différents en fonction des 'keywords'
// passés en entrée: le résultat de la compilation est bien sûr mis en cache
// pour ne pas avoir à les recompiler à chaque frame.
// En combinant cela avec des includes, cela permet de générer automatiquement 
// des shaders pour différents paramètres de scène: fog, ombres, types de lumière, etc.
// Les shaders compilés sont détruits au même moment que l'effect
//
// Exemple:
// Chargement d'un Effect:
//
//		Effect e ("resources/shaders/test.glsl");	// combined source
//
// Compilation d'un shader à partir d'un effect:
//
// 		Effect::Keyword keywords[] = 
//			{ 
//				{ "DIRECTIONAL_LIGHT", "" } 	// signale au shader que les lumières sont directionnelles
//				{ "NO_SHADOW_MAP", "" }			// pas de shadow mapping
//			};
//		CompiledShader *shader = e.compileShader(renderer, 2, keywords);
// 
class Effect
{
public:
	struct Keyword
	{
		std::string define;
		std::string value;
	};

	// nullable
	Effect() = default;
	Effect(
		const char *combinedSourcePath, 
		std::initializer_list<Effect::Keyword> keywords = {})
	{
		loadFromFile(combinedSourcePath, keywords);
	}

	Effect(
		const char *vsPath, 
		const char *fsPath, 
		std::initializer_list<Effect::Keyword> keywords = {})
	{
		loadFromFile(vsPath, fsPath, keywords);
	}

	// noncopyable
	Effect(const Effect &) = delete;
	Effect &operator=(const Effect &) = delete;
	// moveable
	Effect(Effect &&rhs)
	{//TODO
	}
	Effect &operator=(Effect &&)
	{//TODO
	}

	~Effect()
	{//TODO
	}


	enum class ShaderStage
	{
		Vertex,
		Fragment
	};

	CompiledShader *compileShader(
		Renderer &renderer, 
		int numAdditionalKeywords = 0, 
		Keyword *additionalKeywords = nullptr);

	struct Parameter
	{
		std::string name;
		EffectParameterType type;
	};

	void loadFromFile(const char *combinedSourcePath, std::initializer_list<Effect::Keyword> keywords = {});
	void loadFromFile(const char *vsPath, const char *fsPath, std::initializer_list<Effect::Keyword> keywords = {});

private:
	std::size_t getHash(int numAdditionalKeywords, Keyword *additionalKeywords);
	std::string preprocess(
		const boost::filesystem::path &sourcePath, 
		std::istream &sourceIn, 
		int numAdditionalKeywords,
		Effect::Keyword *additionalKeywords,
		ShaderStage stage);
	void preprocessRec(
		const boost::filesystem::path &sourcePath, 
		std::istream &sourceIn, 
		std::ostream &sourceOut, 
		int includeDepth,
		int &glslVersion);

	static unsigned int sCurrentID;
	unsigned int mID = 0; 
	boost::filesystem::path mVSPath;
	boost::filesystem::path mFSPath;
	std::string mVertexShader;
	std::string mFragmentShader;
	// true if mVertexShader contains both the vertex shader and the fragment shader
	bool mCombinedSource;
	// list of #defines
	std::vector<Keyword> mKeywords;
	// list of material parameters
	std::vector<Parameter> mParameters;
	// Render state
	RenderState mRenderState;
	// Shader cache
	std::unordered_map<uint64_t, std::unique_ptr<CompiledShader> > mShaderCache;
};

//=============================================================================
// CompiledShader
// CONTAINS:
// - shader state (VS + PS)
// - render state
// EXCLUDED:
// - sampler states (textures)
// - per-frame constant buffer
// - material parameters (CPU-side)
// - input buffers
// - render targets
class CompiledShader
{
public:
	friend class Effect;

	CompiledShader() = delete;
	// noncopyable
	CompiledShader(const CompiledShader &) = delete;
	CompiledShader &operator=(const CompiledShader &) = delete;
	// nonmoveable
	CompiledShader(CompiledShader &&rhs) = delete;
	CompiledShader &operator=(CompiledShader &&rhs) = delete;

	~CompiledShader()
	{//TODO
	}

	uint64_t getKey() const;
	// setup renderer
	void setup(Renderer &renderer)
	{
		renderer.setShader(mShader);
		// TODO set render states
	}

private:
	CompiledShader(
		Effect &effect,
		uint64_t key,
		Shader *shader,
		const RenderState &renderState);

	// effect
	Effect *mEffect;	// borrowed
	// key 
	uint64_t mKey;
	// Compiled shader (VS+PS)
	Shader *mShader; // owned 
	// Render states (can be overriden by the scene config)
	RenderState mRenderState; 
};


#endif
