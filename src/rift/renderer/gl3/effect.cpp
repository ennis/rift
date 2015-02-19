#include <effect.hpp>
#include <sstream>
#include <algorithm>
#include <vector>
#include <log.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iomanip>

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

	//==================================================================
	//==================================================================
	// GLSL preprocessor
	//==================================================================
	//==================================================================
	enum class GLShaderStage
	{
		Vertex,
		Fragment
	};

	void glslPreprocessRec(
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
			glslPreprocessRec(path, includeStreamIn, sourceOut, includeDepth + 1, glslVersion);
		}
	}

	std::string glslPreprocess(
		const boost::filesystem::path &sourcePath,
		std::istream &sourceIn,
		std::array_ref<Effect::Keyword> keywords,
		std::array_ref<Effect::Keyword> additionalKeywords,
		GLShaderStage stage)
	{
		int glslVersion = 110;
		std::ostringstream tmp;
		glslPreprocessRec(sourcePath, sourceIn, tmp, 0, glslVersion);
		//LOG << "GLSL version " << glslVersion;
		std::ostringstream source;
		source << "#version " << glslVersion << '\n';
		switch (stage) {
		case GLShaderStage::Vertex:
			source << "#define _VERTEX_\n";
			break;
		case GLShaderStage::Fragment:
			source << "#define _FRAGMENT_\n";
			break;
		}
		for (auto &k : keywords) {
			source << "#define " << k.define << " " << k.value << '\n';
		}
		for (auto &k : additionalKeywords) {
			source << "#define " << k.define << " " << k.value << '\n';
		}
		source << tmp.str();
		return source.str();
	}

	std::size_t hashKeywords(
		std::array_ref<Effect::Keyword> keywords, 
		std::array_ref<Effect::Keyword> additionalKeywords)
	{
		std::size_t a = 0;
		std::hash<std::string> hashfn;
		for (const auto &k : additionalKeywords) {
			a += hashfn(k.define);
			a += hashfn(k.value);
		}
		for (const auto &k : keywords) {
			a += hashfn(k.define);
			a += hashfn(k.value);
		}
		return a;
	}
}

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

Shader *Effect::compileShader(
	Renderer &renderer,
	std::array_ref<Keyword> additionalKeywords)
{
	auto hash = hashKeywords(mKeywords, additionalKeywords);
	//LOG << "Hash " << hash;
	auto &cs = mShaderCache[hash];
	if (cs != nullptr) {
		// already compiled
		return cs.get();
	}

	std::ostringstream shader_name;
	shader_name << mID << '/'
		<< std::setw(16) << std::setfill('0') << std::hex << hash << std::dec << '/';
	// log keywords
	for (const auto &k : additionalKeywords) {
		shader_name << k.define;
		if (k.value.size()) {
			shader_name << '=' << k.value;
		}
		shader_name << '/';
	}
	LOG << "Compiling " << shader_name.str();

	std::string vs, fs;
	if (mCombinedSource) {
		// combined source file
		std::istringstream vsIn(mVertexShader);
		// preprocess source file as a vertex shader source
		vs = glslPreprocess(mVSPath, vsIn, std::make_array_ref(mKeywords), additionalKeywords, GLShaderStage::Vertex);
		// rewind stream and parse as a fragment shader source
		vsIn.clear();
		vsIn.seekg(0);
		fs = glslPreprocess(mVSPath, vsIn, std::make_array_ref(mKeywords), additionalKeywords, GLShaderStage::Fragment);
	}
	else {
		// split sources
		// preprocess vertex shader
		std::istringstream vsIn(mVertexShader);
		vs = glslPreprocess(mVSPath, vsIn, std::make_array_ref(mKeywords), additionalKeywords, GLShaderStage::Vertex);
		// preprocess fragment shader
		std::istringstream fsIn(mFragmentShader);
		fs = glslPreprocess(mFSPath, fsIn, std::make_array_ref(mKeywords), additionalKeywords, GLShaderStage::Fragment);
	}

	// create new shader and insert it into the cache
	// move-assignment
	cs = std::unique_ptr<Shader>(new Shader(vs, fs));
	return cs.get();
}

void Effect::loadFromFile(
	const char *combinedSourcePath, 
	std::initializer_list<Effect::Keyword> keywords
	)
{
	mVertexShader = loadShaderSource(combinedSourcePath);
	mCombinedSource = true;
	mVSPath = combinedSourcePath;
	mFSPath = combinedSourcePath;
	mKeywords.assign(keywords);
	mID = sCurrentID++;
}

void Effect::loadFromFile(
	const char *vsPath, 
	const char *fsPath, 
	std::initializer_list<Effect::Keyword> keywords
	)
{
	mVertexShader = loadShaderSource(vsPath);
	mFragmentShader = loadShaderSource(fsPath);
	mCombinedSource = false;
	mVSPath = vsPath;
	mFSPath = fsPath;
	mKeywords.assign(keywords);
	mID = sCurrentID++;
}
