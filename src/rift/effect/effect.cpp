#include <effect.hpp>
#include <sstream>
#include <algorithm>
#include <regex>
#include <vector>

void Material::setup(Renderer &renderer)
{
	for (auto &p : mParameters)
	{
		auto &pp = p.second;
		switch (pp.type)
		{
		case EffectParameterType::Float:
			renderer.setNamedConstantFloat(pp.name.c_str(), pp.u.f32[0]);
			break;
		case EffectParameterType::Float2:
			renderer.setNamedConstantFloat2(pp.name.c_str(), 
				glm::make_vec2(pp.u.f32.data()));
			break;
		case EffectParameterType::Float3:
			renderer.setNamedConstantFloat3(pp.name.c_str(), 
				glm::make_vec3(pp.u.f32.data()));
			break;
		case EffectParameterType::Float4:
			renderer.setNamedConstantFloat4(pp.name.c_str(), 
				glm::make_vec4(pp.u.f32.data()));
			break;
		case EffectParameterType::Int:
			renderer.setNamedConstantInt(pp.name.c_str(), pp.u.i32[0]);
			break;
		case EffectParameterType::Int2:
			renderer.setNamedConstantInt2(pp.name.c_str(), glm::ivec2(pp.u.i32[0], pp.u.i32[1]));
			break;
		case EffectParameterType::Int3:
			//renderer.setNamedConstantInt3(pp.name.c_str(), glm::ivec3(pp.u.i32[0], pp.u.i32[1], pp.u.i32[2]));
			break;
		case EffectParameterType::Int4:
			//renderer.setNamedConstantInt4(pp.name.c_str(), glm::ivec4(pp.u.i32[0], pp.u.i32[1], pp.u.i32[2], pp.u.i32[3]));
			break;
		case EffectParameterType::Float3x4:
			// TODO
			break;
		case EffectParameterType::Float4x4:
			renderer.setNamedConstantMatrix4(pp.name.c_str(), glm::make_mat4(pp.u.f32.data()));
			break;
		default:
			break;
		}
	}

	for (int i = 0; i < 16; ++i)
	{
		renderer.setTexture(i, mSamplers[i]);
	}
}

//=============================================================================
Effect::~Effect()
{}

//=============================================================================
Effect::Effect(
	ShaderSource *vsSource,
	ShaderSource *fsSource,
	RenderState const &renderState)
{
	createFromSource(vsSource, fsSource, renderState);
}

//=============================================================================
void Effect::setEffectID(unsigned int ID)
{
	mID = ID;
}

//=============================================================================
unsigned int Effect::getEffectID() const
{
	return mID;
}

//=============================================================================
void Effect::createFromSource(
	ShaderSource *vsSource, 
	ShaderSource *fsSource,
	RenderState const &renderState)
{
	mVertexShader = vsSource;
	mFragmentShader = fsSource;
	mRenderState = renderState;
}

//=============================================================================
PipelineState::PipelineState(
		EffectCompiler *effectCompiler,
		uint64_t key,
		ConstantBuffer *sceneParamsCS,
		ConstantBuffer *materialParamsCS,
		Shader *compiledShader,
		RenderState const &renderState) :
	mEffectCompiler(effectCompiler),
	mKey(key),
	mSceneParamsCS(sceneParamsCS),
	mMaterialParamsCS(materialParamsCS),
	mShader(compiledShader),
	mRenderState(renderState)
{
	// dummy ctor
}

//=============================================================================
PipelineState::~PipelineState()
{
}

//=============================================================================
void PipelineState::setup()
{
	// TODO bind constant buffers
	auto rd = mEffectCompiler->getRenderer();
	rd->setShader(mShader);
}

//=============================================================================
EffectCompiler::EffectCompiler(Renderer *renderer, const char *includeDir) :
	mRenderer(renderer),
	mIncludeDir(includeDir),
	mIDShaderNode(1),
	mIDEffect(1),
	mIDPipelineState(1)
{
}

//=============================================================================
EffectCompiler::~EffectCompiler()
{
	// all pipeline states in mPipelineStates are automatically destroyed
}

//=============================================================================
void EffectCompiler::registerEffect(Effect *effect)
{
	if (effect->getEffectID() == 0) {
		// effectID == 0 means that the effect is not in the DB
		// get a new effectID and set it
		effect->setEffectID(mIDEffect++);
	}
}

//=============================================================================
static std::vector<std::string> split(const std::string& input, const std::regex& regex) {
	// passing -1 as the submatch index parameter performs splitting
	const std::sregex_token_iterator
		first(input.begin(), input.end(), regex, -1),
		last;
	return{ first, last };
}

//=============================================================================
PipelineState *EffectCompiler::createPipelineState(
	Effect *effect,
	const char *defines)
{
	// TODO check if the configuration is in the cache

	std::ostringstream vsSource;
	std::ostringstream fsSource;
	// insert defines
	auto defs = split(std::string(defines), std::regex(","));
	// remove empty defines
	defs.erase(std::remove(std::begin(defs), std::end(defs), ""), std::end(defs));

	for (auto &&d : defs) {
		vsSource << "#define " << d.c_str() << "\n";
		fsSource << "#define " << d.c_str() << "\n";
	}
	// preprocess vertex and shader sources
	effect->mVertexShader->preprocess(vsSource, mIncludeDir.c_str(), defines);
	effect->mFragmentShader->preprocess(fsSource, mIncludeDir.c_str(), defines);
	LOG << "//====================\n"
		   "// Vertex shader \n"
		   "//====================\n"
		<< vsSource.str();
	LOG << "//====================\n"
		"// Fragment shader \n"
		"//====================\n"
		<< fsSource.str();
	// create shader
	Shader *shader = mRenderer->createShader(
		vsSource.str().c_str(), 
		fsSource.str().c_str());
	// XXX debug
	int shaderBinaryLength;
	auto shaderBinary = shader->getProgramBinary(shaderBinaryLength);
	std::ofstream fileOut("shadercache/out.bin", std::ios::out | std::ios::binary);
	fileOut.write((const char *)shaderBinary.get(), shaderBinaryLength);
	fileOut.close();
	// create pipeline state
	PipelineState *ps = new PipelineState(
		/* effectManager */ this,
		/* key */ 0,
		/* sceneParamsCS */ nullptr,
		/* materialParamsCS */ nullptr,
		/* compiledShader */ shader,
		/* renderState */ effect->mRenderState);
	mPipelineStates.push_back(std::unique_ptr<PipelineState>(ps));
	return ps;
}

//=============================================================================
PipelineState *EffectCompiler::createPipelineStateFromShader(
	// dummy effect
	Effect *effect,
	Shader *shader)
{
	registerEffect(effect);
	PipelineState *ps = new PipelineState(
		/* effectManager */ this,
		/* key */ 0,
		/* sceneParamsCS */ nullptr,
		/* materialParamsCS */ nullptr,
		/* compiledShader */ shader,
		/* renderState */ RenderState());
	mPipelineStates.push_back(std::unique_ptr<PipelineState>(ps));
	return ps;
}
