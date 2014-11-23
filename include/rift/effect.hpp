#ifndef EFFECT_HPP
#define EFFECT_HPP

#include <renderer.hpp>
#include <shadersource.hpp>
#include <map>

class EffectCompiler;

//=============================================================================
class Effect
{
public:
	friend class EffectCompiler;

	Effect() = default;
	Effect(
		ShaderSource *vsSource,
		ShaderSource *fsSource,
		RenderState const &renderState = RenderState());
	virtual ~Effect();

	void createFromSource(
		ShaderSource *vsSource, 
		ShaderSource *fsSource, 
		RenderState const &renderState = RenderState());
	
	// used internally by EffectCompiler
	void setEffectID(unsigned int id);
	unsigned int getEffectID() const;

private:
	// same
	unsigned int mID = 0; 
	// TODO: with effect files, the shaders will be merged into one source file
	// remove the ShaderSource class
	// will contain passes and techniques
	// borrowed refs
	ShaderSource *mVertexShader = nullptr;
	ShaderSource *mGeometryShader = nullptr;
	ShaderSource *mFragmentShader = nullptr;
	// Render state
	RenderState mRenderState;
};

typedef std::unique_ptr<Effect> EffectPtr;

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
	std::vector<PipelineStatePtr> mPipelineStates;
	// Map key -> PipelineState (borrowed)
	std::map<uint64_t, PipelineState*> mPipelineStateMap;

	//
	// Database:
	//
	// EFFECT_ID|Hash(config string) => Pipeline state
};

#endif
