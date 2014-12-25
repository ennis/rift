#include <effect.hpp>
#include <sstream>
#include <algorithm>
#include <vector>
#include <log.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iomanip>

unsigned int Effect::sCurrentID = 0;

//=============================================================================
std::string loadShaderSource(const char *fileName)
{
	std::ifstream fileIn(fileName, std::ios::in);
	if (!fileIn.is_open()) {
		ERROR << "Could not open shader file " << fileName;
		throw std::runtime_error("Could not open shader file");
	}
	std::string str;
	str.assign(
		(std::istreambuf_iterator<char>(fileIn)),
		std::istreambuf_iterator<char>());
	return str;
}

CompiledShader *Effect::compileShader(
	Renderer &renderer,
	int numAdditionalKeywords, 
	Effect::Keyword *additionalKeywords)
{
	auto hash = getHash(numAdditionalKeywords, additionalKeywords);
	//LOG << "Hash " << hash;
	auto &cs = mShaderCache[hash];
	if (cs != nullptr) {
		// already compiled
		return cs.get();
	}

	std::ostringstream shader_name;
	shader_name << mID << '/'
		<< std::setw(16) << std::hex << hash << std::dec << '/';
	for (int i = 0; i < numAdditionalKeywords; ++i) {
		shader_name << additionalKeywords[i].define;
		if (additionalKeywords[i].value.size()) {
			shader_name << '=' << additionalKeywords[i].value;
		}
		shader_name << '/';
	}
	LOG << "Compiling " << shader_name.str();

	std::string vs, fs;
	if (mCombinedSource) {
		std::istringstream vsIn(mVertexShader);
		vs = preprocess(mVSPath, vsIn, numAdditionalKeywords, additionalKeywords, ShaderStage::Vertex);
		vsIn.clear();
		vsIn.seekg(0);
		fs = preprocess(mVSPath, vsIn, numAdditionalKeywords, additionalKeywords, ShaderStage::Fragment);
	}
	else {
		std::istringstream vsIn(mVertexShader);
		vs = preprocess(mVSPath, vsIn, numAdditionalKeywords, additionalKeywords, ShaderStage::Vertex);
		std::istringstream fsIn(mFragmentShader);
		fs = preprocess(mFSPath, fsIn, numAdditionalKeywords, additionalKeywords, ShaderStage::Fragment);
	}
	// create shader
	LOG << vs;
	LOG << fs;
	auto sh = renderer.createShader(vs.c_str(), fs.c_str());
	cs = std::unique_ptr<CompiledShader>(new CompiledShader(*this, hash, sh, RenderState()));
	return cs.get();
}

std::size_t Effect::getHash(int numAdditionalKeywords, Effect::Keyword *additionalKeywords)
{
	std::size_t a = 0;
	std::hash<std::string> hashfn;
	for (int i =0; i < numAdditionalKeywords; ++i) {
		a += hashfn(additionalKeywords[i].define);
		a += hashfn(additionalKeywords[i].value);
	}
	for (auto &k : mKeywords) {
		a += hashfn(k.define);
		a += hashfn(k.value);
	}
	return a;
}

namespace
{
	std::locale cloc("C");

	template <typename Fn>
	void skip(const std::string &line, int &pos, Fn pred)
	{
		// skip whitespace
		while (pred(line[pos], cloc) && pos < line.size()) { pos++; }
	}

	bool path_character(char ch, const std::locale &loc)
	{
		return ch != '>';
	}

	std::string next_word(const std::string &line, int &pos)
	{
		// skip whitespace
		skip(line, pos, std::isspace<char>);
		int beg = pos;
		// get word
		skip(line, pos, std::isalnum<char>);
		return line.substr(beg, pos - beg);
	}

	int parse_glsl_version(const std::string &line, int &pos)
	{
		int end = line.size();
		// skip whitespace
		skip(line, pos, std::isspace<char>);
		// version number
		int beg = pos;
		skip(line, pos, std::isdigit<char>);
		auto version = line.substr(beg, pos - beg);
		// skip whitespace
		skip(line, pos, std::isspace<char>);
		// must reach end of line
		if (pos != end) return 0;
		return std::stoi(version);
	}

	std::string parse_include_path(const std::string &line, int &pos)
	{
		int end = line.size();
		// skip whitespace
		skip(line, pos, std::isspace<char>);
		if (pos == end || line[pos++] != '<') return "";
		int beg = pos;
		skip(line, pos, path_character);
		auto path = line.substr(beg, pos - beg);
		// closing '<'
		if (pos == end || line[pos++] != '>') return "";
		// skip whitespace
		skip(line, pos, std::isspace<char>);
		// must reach end of line
		if (pos != end) return "";
		return path;
	}
}

std::string Effect::preprocess(
	const boost::filesystem::path &sourcePath, 
	std::istream &sourceIn, 
	int numAdditionalKeywords,
	Effect::Keyword *additionalKeywords,
	ShaderStage stage)
{
	int glslVersion = 110;
	std::ostringstream tmp;
	preprocessRec(sourcePath, sourceIn, tmp, 0, glslVersion);
	//LOG << "GLSL version " << glslVersion;
	std::ostringstream source;
	source << "#version " << glslVersion << '\n';
	switch (stage) {
	case ShaderStage::Vertex:
		source << "#define _VERTEX_\n";
		break;
	case ShaderStage::Fragment:
		source << "#define _FRAGMENT_\n";
		break;
	}
	for (auto &k : mKeywords) {
		source << "#define " << k.define << " " << k.value << '\n';
	}
	for (int i = 0; i < numAdditionalKeywords; ++i) {
		source << "#define " << additionalKeywords[i].define << " " << additionalKeywords[i].value << '\n';
	}
	source << tmp.str();
	return source.str();
}

void Effect::preprocessRec(
	const boost::filesystem::path &sourcePath, 
	std::istream &sourceIn, 
	std::ostream &sourceOut, 
	int includeDepth,
	int &glslVersion)
{
	using namespace boost::filesystem;
	
	const path systemIncludePath("resources/shaders/include");
	const int kMaxIncludeDepth = 20;
	
	if (includeDepth > kMaxIncludeDepth) {
		ERROR << "Maximum include depth reached (circular include?)";
		return;
	}

	std::string line;
	while (std::getline(sourceIn, line)) {
		// if the first character of the line is a '#', then it is a preprocessing directive
		if (line[0] != '#') {
			sourceOut << line << '\n';
			continue;
		}
		int pos = 1;
		std::string directive = next_word(line, pos);
		if (directive == "version") {
			glslVersion = parse_glsl_version(line, pos);
			continue;
		}
		if (directive != "pragma") {
			sourceOut << line << '\n';
			continue;
		}
		std::string action = next_word(line, pos);
		if (action != "include") {
			sourceOut << line << '\n';
			continue;
		}
		auto include = parse_include_path(line, pos);
		if (include.size() == 0) {
			WARNING << "Parse error in #pragma include";
			sourceOut << line << '\n';
			continue;
		}

		// look for include file in source path
		auto path = sourcePath.parent_path();
		path /= include;
		LOG << "include " << path.string();
		if (!exists(path)) {
			// does not exist, look in system include dir
			path = systemIncludePath;
			path /= include;
			LOG << "include (sys) " << path.string();
			if (!exists(path)) {
				//
				ERROR << "Include file <" << include << "> not found.";
				continue;
			}
		}
		// put marker in processed source
		sourceOut << "// Include file " << path.string() << " from " << sourcePath.string() << '\n';
		// open file
		std::ifstream includeStreamIn(path.c_str(), std::ios::in);
		preprocessRec(path, includeStreamIn, sourceOut, includeDepth + 1, glslVersion);
	}
}

void Effect::loadFromFile(const char *combinedSourcePath, std::initializer_list<Effect::Keyword> keywords)
{
	mVertexShader = loadShaderSource(combinedSourcePath);
	mCombinedSource = true;
	mVSPath = combinedSourcePath;
	mFSPath = combinedSourcePath;
	mKeywords.assign(keywords);
	mID = sCurrentID++;
}

void Effect::loadFromFile(const char *vsPath, const char *fsPath, std::initializer_list<Effect::Keyword> keywords)
{
	mVertexShader = loadShaderSource(vsPath);
	mFragmentShader = loadShaderSource(fsPath);
	mCombinedSource = false;
	mVSPath = vsPath;
	mFSPath = fsPath;
	mKeywords.assign(keywords);
	mID = sCurrentID++;
}


CompiledShader::CompiledShader(
	Effect &effect,
	uint64_t key,
	Shader *shader,
	const RenderState &renderState) :
mEffect(&effect),
mKey(key),
mShader(shader),
mRenderState(renderState)
{

}