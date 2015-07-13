#include <Include/ShHandle.h>
#include <Public/ShaderLang.h>
#include <Include/intermediate.h>
#include <MachineIndependent/localintermediate.h>
#include "getopt_pp.h"
#include <iostream>
#include <filesystem>
#include <memory>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include "IncludePreprocessor.hpp"
#include "main.h"

TBuiltInResource Resources;

std::string LoadShaderSource(const char *fileName)
{
	using namespace std;
	ifstream fileIn(fileName, ios::in);
	if (!fileIn.is_open()) {
		cerr << "Could not open shader file " << fileName << endl; 
		assert(false);
	}
	string str;
	str.assign(
		(istreambuf_iterator<char>(fileIn)),
		istreambuf_iterator<char>());
	return str;
}

//
// Parse either a .conf file provided by the user or the default string above.
//
//
// These are the default resources for TBuiltInResources, used for both
//  - parsing this string for the case where the user didn't supply one
//  - dumping out a template for user construction of a config file
//
const char* DefaultConfig =
"MaxLights 32\n"
"MaxClipPlanes 6\n"
"MaxTextureUnits 32\n"
"MaxTextureCoords 32\n"
"MaxVertexAttribs 64\n"
"MaxVertexUniformComponents 4096\n"
"MaxVaryingFloats 64\n"
"MaxVertexTextureImageUnits 32\n"
"MaxCombinedTextureImageUnits 80\n"
"MaxTextureImageUnits 32\n"
"MaxFragmentUniformComponents 4096\n"
"MaxDrawBuffers 32\n"
"MaxVertexUniformVectors 128\n"
"MaxVaryingVectors 8\n"
"MaxFragmentUniformVectors 16\n"
"MaxVertexOutputVectors 16\n"
"MaxFragmentInputVectors 15\n"
"MinProgramTexelOffset -8\n"
"MaxProgramTexelOffset 7\n"
"MaxClipDistances 8\n"
"MaxComputeWorkGroupCountX 65535\n"
"MaxComputeWorkGroupCountY 65535\n"
"MaxComputeWorkGroupCountZ 65535\n"
"MaxComputeWorkGroupSizeX 1024\n"
"MaxComputeWorkGroupSizeY 1024\n"
"MaxComputeWorkGroupSizeZ 64\n"
"MaxComputeUniformComponents 1024\n"
"MaxComputeTextureImageUnits 16\n"
"MaxComputeImageUniforms 8\n"
"MaxComputeAtomicCounters 8\n"
"MaxComputeAtomicCounterBuffers 1\n"
"MaxVaryingComponents 60\n"
"MaxVertexOutputComponents 64\n"
"MaxGeometryInputComponents 64\n"
"MaxGeometryOutputComponents 128\n"
"MaxFragmentInputComponents 128\n"
"MaxImageUnits 8\n"
"MaxCombinedImageUnitsAndFragmentOutputs 8\n"
"MaxCombinedShaderOutputResources 8\n"
"MaxImageSamples 0\n"
"MaxVertexImageUniforms 0\n"
"MaxTessControlImageUniforms 0\n"
"MaxTessEvaluationImageUniforms 0\n"
"MaxGeometryImageUniforms 0\n"
"MaxFragmentImageUniforms 8\n"
"MaxCombinedImageUniforms 8\n"
"MaxGeometryTextureImageUnits 16\n"
"MaxGeometryOutputVertices 256\n"
"MaxGeometryTotalOutputComponents 1024\n"
"MaxGeometryUniformComponents 1024\n"
"MaxGeometryVaryingComponents 64\n"
"MaxTessControlInputComponents 128\n"
"MaxTessControlOutputComponents 128\n"
"MaxTessControlTextureImageUnits 16\n"
"MaxTessControlUniformComponents 1024\n"
"MaxTessControlTotalOutputComponents 4096\n"
"MaxTessEvaluationInputComponents 128\n"
"MaxTessEvaluationOutputComponents 128\n"
"MaxTessEvaluationTextureImageUnits 16\n"
"MaxTessEvaluationUniformComponents 1024\n"
"MaxTessPatchComponents 120\n"
"MaxPatchVertices 32\n"
"MaxTessGenLevel 64\n"
"MaxViewports 16\n"
"MaxVertexAtomicCounters 0\n"
"MaxTessControlAtomicCounters 0\n"
"MaxTessEvaluationAtomicCounters 0\n"
"MaxGeometryAtomicCounters 0\n"
"MaxFragmentAtomicCounters 8\n"
"MaxCombinedAtomicCounters 8\n"
"MaxAtomicCounterBindings 1\n"
"MaxVertexAtomicCounterBuffers 0\n"
"MaxTessControlAtomicCounterBuffers 0\n"
"MaxTessEvaluationAtomicCounterBuffers 0\n"
"MaxGeometryAtomicCounterBuffers 0\n"
"MaxFragmentAtomicCounterBuffers 1\n"
"MaxCombinedAtomicCounterBuffers 1\n"
"MaxAtomicCounterBufferSize 16384\n"
"MaxTransformFeedbackBuffers 4\n"
"MaxTransformFeedbackInterleavedComponents 64\n"
"MaxCullDistances 8\n"
"MaxCombinedClipAndCullDistances 8\n"
"MaxSamples 4\n"
"nonInductiveForLoops 1\n"
"whileLoops 1\n"
"doWhileLoops 1\n"
"generalUniformIndexing 1\n"
"generalAttributeMatrixVectorIndexing 1\n"
"generalVaryingIndexing 1\n"
"generalSamplerIndexing 1\n"
"generalVariableIndexing 1\n"
"generalConstantMatrixVectorIndexing 1\n"
;

std::string split(const std::string &in, int &pos, const char *delim)
{
	pos = in.find_first_not_of(delim, pos);
	if (pos == std::string::npos)
		return std::string();
	int startpos = pos;
	pos = in.find_first_of(delim, pos);
	if (pos == std::string::npos)
		return in.substr(startpos);
	return in.substr(startpos, pos - startpos);
}

void ProcessConfigFile(std::string configFile)
{
	std::string config;
	if (configFile.size() > 0) 
	{
		// Load config file
		std::ifstream fileIn(configFile, std::ios::in);
		if (!fileIn.is_open()) 
		{
			std::cerr << "Error opening configuration file; will instead use the default configuration\n";
		}
		else
		{
			config.assign(
				(std::istreambuf_iterator<char>(fileIn)),
				std::istreambuf_iterator<char>());
		}
	}

	if (config.size() == 0)
	{
		config.assign(DefaultConfig);
	}

	const char* delims = " \t\n\r";
	int pos = 0;
	std::string token = split(config, pos, delims);
	while (token.size()) {
		std::string valueStr = split(config, pos, delims);
		if (valueStr.size() == 0 || !(valueStr[0] == '-' || (valueStr[0] >= '0' && valueStr[0] <= '9'))) {
			printf("Error: '%s' bad .conf file.  Each name must be followed by one number.\n", valueStr.size() ? valueStr.c_str() : "");
			return;
		}
		int value = std::stoi(valueStr);

		if (token.compare("MaxLights") == 0)
			Resources.maxLights = value;
		else if (token.compare("MaxClipPlanes") == 0)
			Resources.maxClipPlanes = value;
		else if (token.compare("MaxTextureUnits") == 0)
			Resources.maxTextureUnits = value;
		else if (token.compare("MaxTextureCoords") == 0)
			Resources.maxTextureCoords = value;
		else if (token.compare("MaxVertexAttribs") == 0)
			Resources.maxVertexAttribs = value;
		else if (token.compare("MaxVertexUniformComponents") == 0)
			Resources.maxVertexUniformComponents = value;
		else if (token.compare("MaxVaryingFloats") == 0)
			Resources.maxVaryingFloats = value;
		else if (token.compare("MaxVertexTextureImageUnits") == 0)
			Resources.maxVertexTextureImageUnits = value;
		else if (token.compare("MaxCombinedTextureImageUnits") == 0)
			Resources.maxCombinedTextureImageUnits = value;
		else if (token.compare("MaxTextureImageUnits") == 0)
			Resources.maxTextureImageUnits = value;
		else if (token.compare("MaxFragmentUniformComponents") == 0)
			Resources.maxFragmentUniformComponents = value;
		else if (token.compare("MaxDrawBuffers") == 0)
			Resources.maxDrawBuffers = value;
		else if (token.compare("MaxVertexUniformVectors") == 0)
			Resources.maxVertexUniformVectors = value;
		else if (token.compare("MaxVaryingVectors") == 0)
			Resources.maxVaryingVectors = value;
		else if (token.compare("MaxFragmentUniformVectors") == 0)
			Resources.maxFragmentUniformVectors = value;
		else if (token.compare("MaxVertexOutputVectors") == 0)
			Resources.maxVertexOutputVectors = value;
		else if (token.compare("MaxFragmentInputVectors") == 0)
			Resources.maxFragmentInputVectors = value;
		else if (token.compare("MinProgramTexelOffset") == 0)
			Resources.minProgramTexelOffset = value;
		else if (token.compare("MaxProgramTexelOffset") == 0)
			Resources.maxProgramTexelOffset = value;
		else if (token.compare("MaxClipDistances") == 0)
			Resources.maxClipDistances = value;
		else if (token.compare("MaxComputeWorkGroupCountX") == 0)
			Resources.maxComputeWorkGroupCountX = value;
		else if (token.compare("MaxComputeWorkGroupCountY") == 0)
			Resources.maxComputeWorkGroupCountY = value;
		else if (token.compare("MaxComputeWorkGroupCountZ") == 0)
			Resources.maxComputeWorkGroupCountZ = value;
		else if (token.compare("MaxComputeWorkGroupSizeX") == 0)
			Resources.maxComputeWorkGroupSizeX = value;
		else if (token.compare("MaxComputeWorkGroupSizeY") == 0)
			Resources.maxComputeWorkGroupSizeY = value;
		else if (token.compare("MaxComputeWorkGroupSizeZ") == 0)
			Resources.maxComputeWorkGroupSizeZ = value;
		else if (token.compare("MaxComputeUniformComponents") == 0)
			Resources.maxComputeUniformComponents = value;
		else if (token.compare("MaxComputeTextureImageUnits") == 0)
			Resources.maxComputeTextureImageUnits = value;
		else if (token.compare("MaxComputeImageUniforms") == 0)
			Resources.maxComputeImageUniforms = value;
		else if (token.compare("MaxComputeAtomicCounters") == 0)
			Resources.maxComputeAtomicCounters = value;
		else if (token.compare("MaxComputeAtomicCounterBuffers") == 0)
			Resources.maxComputeAtomicCounterBuffers = value;
		else if (token.compare("MaxVaryingComponents") == 0)
			Resources.maxVaryingComponents = value;
		else if (token.compare("MaxVertexOutputComponents") == 0)
			Resources.maxVertexOutputComponents = value;
		else if (token.compare("MaxGeometryInputComponents") == 0)
			Resources.maxGeometryInputComponents = value;
		else if (token.compare("MaxGeometryOutputComponents") == 0)
			Resources.maxGeometryOutputComponents = value;
		else if (token.compare("MaxFragmentInputComponents") == 0)
			Resources.maxFragmentInputComponents = value;
		else if (token.compare("MaxImageUnits") == 0)
			Resources.maxImageUnits = value;
		else if (token.compare("MaxCombinedImageUnitsAndFragmentOutputs") == 0)
			Resources.maxCombinedImageUnitsAndFragmentOutputs = value;
		else if (token.compare("MaxCombinedShaderOutputResources") == 0)
			Resources.maxCombinedShaderOutputResources = value;
		else if (token.compare("MaxImageSamples") == 0)
			Resources.maxImageSamples = value;
		else if (token.compare("MaxVertexImageUniforms") == 0)
			Resources.maxVertexImageUniforms = value;
		else if (token.compare("MaxTessControlImageUniforms") == 0)
			Resources.maxTessControlImageUniforms = value;
		else if (token.compare("MaxTessEvaluationImageUniforms") == 0)
			Resources.maxTessEvaluationImageUniforms = value;
		else if (token.compare("MaxGeometryImageUniforms") == 0)
			Resources.maxGeometryImageUniforms = value;
		else if (token.compare("MaxFragmentImageUniforms") == 0)
			Resources.maxFragmentImageUniforms = value;
		else if (token.compare("MaxCombinedImageUniforms") == 0)
			Resources.maxCombinedImageUniforms = value;
		else if (token.compare("MaxGeometryTextureImageUnits") == 0)
			Resources.maxGeometryTextureImageUnits = value;
		else if (token.compare("MaxGeometryOutputVertices") == 0)
			Resources.maxGeometryOutputVertices = value;
		else if (token.compare("MaxGeometryTotalOutputComponents") == 0)
			Resources.maxGeometryTotalOutputComponents = value;
		else if (token.compare("MaxGeometryUniformComponents") == 0)
			Resources.maxGeometryUniformComponents = value;
		else if (token.compare("MaxGeometryVaryingComponents") == 0)
			Resources.maxGeometryVaryingComponents = value;
		else if (token.compare("MaxTessControlInputComponents") == 0)
			Resources.maxTessControlInputComponents = value;
		else if (token.compare("MaxTessControlOutputComponents") == 0)
			Resources.maxTessControlOutputComponents = value;
		else if (token.compare("MaxTessControlTextureImageUnits") == 0)
			Resources.maxTessControlTextureImageUnits = value;
		else if (token.compare("MaxTessControlUniformComponents") == 0)
			Resources.maxTessControlUniformComponents = value;
		else if (token.compare("MaxTessControlTotalOutputComponents") == 0)
			Resources.maxTessControlTotalOutputComponents = value;
		else if (token.compare("MaxTessEvaluationInputComponents") == 0)
			Resources.maxTessEvaluationInputComponents = value;
		else if (token.compare("MaxTessEvaluationOutputComponents") == 0)
			Resources.maxTessEvaluationOutputComponents = value;
		else if (token.compare("MaxTessEvaluationTextureImageUnits") == 0)
			Resources.maxTessEvaluationTextureImageUnits = value;
		else if (token.compare("MaxTessEvaluationUniformComponents") == 0)
			Resources.maxTessEvaluationUniformComponents = value;
		else if (token.compare("MaxTessPatchComponents") == 0)
			Resources.maxTessPatchComponents = value;
		else if (token.compare("MaxPatchVertices") == 0)
			Resources.maxPatchVertices = value;
		else if (token.compare("MaxTessGenLevel") == 0)
			Resources.maxTessGenLevel = value;
		else if (token.compare("MaxViewports") == 0)
			Resources.maxViewports = value;
		else if (token.compare("MaxVertexAtomicCounters") == 0)
			Resources.maxVertexAtomicCounters = value;
		else if (token.compare("MaxTessControlAtomicCounters") == 0)
			Resources.maxTessControlAtomicCounters = value;
		else if (token.compare("MaxTessEvaluationAtomicCounters") == 0)
			Resources.maxTessEvaluationAtomicCounters = value;
		else if (token.compare("MaxGeometryAtomicCounters") == 0)
			Resources.maxGeometryAtomicCounters = value;
		else if (token.compare("MaxFragmentAtomicCounters") == 0)
			Resources.maxFragmentAtomicCounters = value;
		else if (token.compare("MaxCombinedAtomicCounters") == 0)
			Resources.maxCombinedAtomicCounters = value;
		else if (token.compare("MaxAtomicCounterBindings") == 0)
			Resources.maxAtomicCounterBindings = value;
		else if (token.compare("MaxVertexAtomicCounterBuffers") == 0)
			Resources.maxVertexAtomicCounterBuffers = value;
		else if (token.compare("MaxTessControlAtomicCounterBuffers") == 0)
			Resources.maxTessControlAtomicCounterBuffers = value;
		else if (token.compare("MaxTessEvaluationAtomicCounterBuffers") == 0)
			Resources.maxTessEvaluationAtomicCounterBuffers = value;
		else if (token.compare("MaxGeometryAtomicCounterBuffers") == 0)
			Resources.maxGeometryAtomicCounterBuffers = value;
		else if (token.compare("MaxFragmentAtomicCounterBuffers") == 0)
			Resources.maxFragmentAtomicCounterBuffers = value;
		else if (token.compare("MaxCombinedAtomicCounterBuffers") == 0)
			Resources.maxCombinedAtomicCounterBuffers = value;
		else if (token.compare("MaxAtomicCounterBufferSize") == 0)
			Resources.maxAtomicCounterBufferSize = value;
		else if (token.compare("MaxTransformFeedbackBuffers") == 0)
			Resources.maxTransformFeedbackBuffers = value;
		else if (token.compare("MaxTransformFeedbackInterleavedComponents") == 0)
			Resources.maxTransformFeedbackInterleavedComponents = value;
		else if (token.compare("MaxCullDistances") == 0)
			Resources.maxCullDistances = value;
		else if (token.compare("MaxCombinedClipAndCullDistances") == 0)
			Resources.maxCombinedClipAndCullDistances = value;
		else if (token.compare("MaxSamples") == 0)
			Resources.maxSamples = value;

		else if (token.compare("nonInductiveForLoops") == 0)
			Resources.limits.nonInductiveForLoops = (value != 0);
		else if (token.compare("whileLoops") == 0)
			Resources.limits.whileLoops = (value != 0);
		else if (token.compare("doWhileLoops") == 0)
			Resources.limits.doWhileLoops = (value != 0);
		else if (token.compare("generalUniformIndexing") == 0)
			Resources.limits.generalUniformIndexing = (value != 0);
		else if (token.compare("generalAttributeMatrixVectorIndexing") == 0)
			Resources.limits.generalAttributeMatrixVectorIndexing = (value != 0);
		else if (token.compare("generalVaryingIndexing") == 0)
			Resources.limits.generalVaryingIndexing = (value != 0);
		else if (token.compare("generalSamplerIndexing") == 0)
			Resources.limits.generalSamplerIndexing = (value != 0);
		else if (token.compare("generalVariableIndexing") == 0)
			Resources.limits.generalVariableIndexing = (value != 0);
		else if (token.compare("generalConstantMatrixVectorIndexing") == 0)
			Resources.limits.generalConstantMatrixVectorIndexing = (value != 0);
		else
			printf("Warning: unrecognized limit (%s) in configuration file.\n", token);
		
		token = split(config, pos, delims);
	}

}

int main(int argc, char* argv[])
{
	using namespace GetOpt;
	using namespace std;
	namespace fs = std::experimental::filesystem;

	bool hasCombinedSource;
	string combinedSourcePath;
	bool hasFragmentShader;
	string fragmentShaderPath;
	bool hasVertexShader;
	string vertexShaderPath;
	string includePath;
	bool hasOutPath;
	string outPath;
	string outVertPath;
	string outFragPath;
	bool verbose = false;


	// Get and sanitize input from command line
	GetOpt_pp ops(argc, argv);
	ops >> Option("combinedSource", combinedSourcePath, "");
	hasCombinedSource = combinedSourcePath.size() != 0;
	ops >> Option("vertexShader", vertexShaderPath, "");
	hasVertexShader = vertexShaderPath.size() != 0;
	ops >> Option("fragmentShader", fragmentShaderPath, "");
	hasFragmentShader = fragmentShaderPath.size() != 0;
	ops >> Option('i', "include", includePath, ".");
	hasOutPath = (bool)(ops >> Option('o', "out", outPath));
	ops >> OptionPresent('-v', verbose);

	fs::path inPath(combinedSourcePath);
	string sourceStem = inPath.stem().string();
	fs::path tmpOutPath;
	fs::path tmpOutVertPath;
	fs::path tmpOutFragPath;
	if (!hasOutPath) 
	{
		tmpOutVertPath = inPath.parent_path();
		tmpOutFragPath = tmpOutVertPath;
	}
	else
	{
		tmpOutVertPath = outPath;
		tmpOutFragPath = outPath;
	}

	tmpOutVertPath.append(sourceStem + ".pp.vert");
	tmpOutFragPath.append(sourceStem + ".pp.frag");
	outVertPath = tmpOutVertPath.string();
	outFragPath = tmpOutFragPath.string();

	using namespace glslang;
	if (verbose)
	{
		cout << "* Combined source file   : " << combinedSourcePath << endl;
		cout << "* Shader include path    : " << includePath << endl;
		cout << "* Output vertex shader   : " << outVertPath << endl;
		cout << "* Output fragment shader : " << outFragPath << endl;
		cout << "* glslang version        : " << GetGlslVersionString() << endl;
	}
	// Begin compiling
	cout << endl;
	InitializeProcess();
	ProcessConfigFile("");

	// create shader & program objects
	unique_ptr<TShader> vsh = make_unique<TShader>(EShLangVertex);
	unique_ptr<TShader> fsh = make_unique<TShader>(EShLangFragment);
	unique_ptr<TProgram> prog = make_unique<TProgram>();

	// Load input file & preprocess include files
	std::vector<std::string> includePaths = { ".", "include", includePath };
	auto combinedStr = LoadShaderSource(combinedSourcePath.c_str());
	std::ostringstream ppiStrOut;
	bool ppSuccess = ProcessIncludes(combinedSourcePath.c_str(), includePaths, ppiStrOut, cout);
	combinedStr = ppiStrOut.str();
	const char *strings[1] = { combinedStr.c_str() };
	if (!ppSuccess) 
	{
		cout << endl;
		cout << "Errors while processing include files. No shaders generated." << endl;
		cout << endl;
	}
	else
	{
		// Compile vertex shader
		if (verbose) cout << "COMPILING VERTEX SHADER" << endl;
		vsh->setStrings(strings, 1);
		vsh->setPreamble("#define _VERTEX_\n");
		bool vertSuccess = vsh->parse(&Resources, 430, ECoreProfile, true, true, EShMsgAST);
		// TODO parse info log?
		cout << vsh->getInfoLog();

		// Compile fragment shader
		if (verbose) cout << "COMPILING FRAGMENT SHADER" << endl;
		fsh->setStrings(strings, 1);
		fsh->setPreamble("#define _FRAGMENT_\n");
		bool fragSuccess = fsh->parse(&Resources, 430, ECoreProfile, true, true, EShMsgAST);
		cout << fsh->getInfoLog();

		bool linkSuccess = false;
		if (vertSuccess && fragSuccess)
		{
			prog->addShader(vsh.get());
			prog->addShader(fsh.get());
			linkSuccess = prog->link((EShMessages)(EShMsgAST));
			cout << prog->getInfoLog();
			prog->buildReflection();
			prog->dumpReflection();
		}

		if (!linkSuccess)
		{
			cout << endl;
			cerr << "Compilation failed. No shaders generated." << endl;
			cout << endl;
		}
		else
		{
			// All OK, open output .vert & .frag files
			// and dump the preprocessed source in it
			// TODO: do not re-run the preprocessor
			if (verbose) cout << "WRITING SHADER FILES" << endl;
			ofstream vertOut(outVertPath);
			ofstream fragOut(outFragPath);
			string ppVert, ppFrag;
			vsh->preprocess(&Resources, 440, ECoreProfile, false, true, EShMsgDefault, &ppVert);
			fsh->preprocess(&Resources, 440, ECoreProfile, false, true, EShMsgDefault, &ppFrag);
			vertOut << ppVert;
			fragOut << ppFrag;
			if (verbose) cout << "DONE" << endl;
		}
	}

	prog.release();
	vsh.release();
	fsh.release();

	glslang::FinalizeProcess();
}