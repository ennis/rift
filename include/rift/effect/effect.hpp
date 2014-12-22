#ifndef EFFECT_HPP
#define EFFECT_HPP

#include <renderer.hpp>
#include <shadersource.hpp>
#include <unordered_map>
#include <array>
#include <glm/gtc/type_ptr.hpp>

class EffectCompiler;

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

//=============================================================================
class Effect
{
public:
	friend class EffectCompiler;

	Effect() = default;
	Effect(
		std::string vertexShaderSource,
		std::string fragmentShaderSource,
		RenderState const &renderState = RenderState());

	~Effect();

	// used internally by EffectCompiler
	void setEffectID(unsigned int id);
	unsigned int getEffectID() const;

	struct Parameter
	{
		std::string name;
		EffectParameterType type;
	};

private:
	unsigned int mID = 0; 

	std::string mVertexShader;
	std::string mFragmentShader;
	// true if mVertexShader contains both the vertex shader and the fragment shader
	bool mCombinedSource;
	// Sorted list of #defines
	std::vector<std::string> mKeywords;
	// list of material parameters
	std::vector<Parameter> mParameters;
	// Render state
	RenderState mRenderState;
};


//=============================================================================
// Pipeline state
// CONTAINS:
// - shader state (VS + PS)
// - render state
// EXCLUDED:
// - sampler states (textures)
// - per-frame constant buffer
// - material parameters (CPU-side)
// - input buffers
// - render targets

class PipelineState
{
public:
	friend class EffectCompiler;
	~PipelineState();

	uint64_t getKey() const;
	// setup renderer
	void setup();

private:
	PipelineState(
		EffectCompiler *effectCompiler,
		uint64_t key,
		ConstantBuffer *sceneParamsCS,
		ConstantBuffer *materialParamsCS,
		Shader *compiledShader,
		RenderState const &renderState);

	// EffectManager instance that created the object
	EffectCompiler *mEffectCompiler;	// borrowed
	// pipeline state key 
	uint64_t mKey;
	// constant buffer for scene parameters (lights, shadows)
	ConstantBuffer *mSceneParamsCS; // borrowed
	// constant buffer for material parameters (those defined in the effect file)
	ConstantBuffer *mMaterialParamsCS; // borrowed

	// Compiled shader (VS+PS)
	Shader *mShader; // owned unique_ptr
	// Render states (can be overriden by the scene config)
	RenderState mRenderState; 
};

typedef std::unique_ptr<PipelineState> PipelineStatePtr;


//=============================================================================
class EffectCompiler
{
public:
	// TODO reference
	EffectCompiler(Renderer *renderer, const char *includeDir);
	~EffectCompiler();

	// createPipelineState needs:
	// - reference to effect instance
	// - configuration string (comma-separated list of list of #defines)
	PipelineState *createPipelineState(
		Effect *effect,
		const char *defines = "");
	// create a pipeline state from a compiled shader
	PipelineState *createPipelineStateFromShader(
		// dummy effect
		Effect *effect,
		Shader *shader);
	// returns the renderer object used to create the resources
	Renderer *getRenderer() const {
		return mRenderer;
	}

private:
	void registerEffect(Effect *effect);

	Renderer *mRenderer; // borrowed
	std::string mIncludeDir;
	// TODO source skeletons for deferred shading

	unsigned int mIDShaderNode;
	unsigned int mIDEffect;
	unsigned int mIDPipelineState;

	// array of pipeline states
	std::vector<PipelineState> mPipelineStates;
	// Map key -> PipelineState (borrowed)
	std::unordered_map<uint64_t, PipelineState*> mPipelineStateMap;

	//
	// Database:
	//
	// EFFECT_ID|Hash(config string) => Pipeline state
};

#endif
