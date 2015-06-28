#include <rendering/opengl4.hpp>
#include <log.hpp>
#include <sstream>
#include <algorithm>
#include <vector>
#include <log.hpp>
#include <fstream>
#include <iomanip>
#include <array_ref.hpp>
#include <boost/filesystem.hpp>

namespace {

	//=========================================================================
	//=========================================================================
	// GLSL preprocessor
	//=========================================================================
	//=========================================================================
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
			if (!exists(path)) {
				// does not exist, look in system include dir
				path = systemIncludePath;
				path /= include;
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
		GLenum stage,
		util::array_ref<ShaderKeyword> keywords)
	{
		int glslVersion = 110;
		std::ostringstream tmp;
		glslPreprocessRec(sourcePath, sourceIn, tmp, 0, glslVersion);
		//LOG << "GLSL version " << glslVersion;
		std::ostringstream source;
		source << "#version " << glslVersion << '\n';
		switch (stage) {
		case gl::VERTEX_SHADER:
			source << "#define _VERTEX_\n";
			break;
		case gl::FRAGMENT_SHADER:
			source << "#define _FRAGMENT_\n";
			break;
		case gl::GEOMETRY_SHADER:
			source << "#define _GEOMETRY_\n";
			break;
		}
		for (auto &k : keywords) {
			source << "#define " << k.define << " " << k.value << '\n';
		}
		source << tmp.str();
		return source.str();
	}

	std::size_t hashKeywords(
		util::array_ref<ShaderKeyword> keywords)
	{
		std::size_t a = 0;
		std::hash<std::string> hashfn;
		for (const auto &k : keywords) {
			a += hashfn(k.define);
			a += hashfn(k.value);
		}
		return a;
	}

	int sCurrentEffectCacheID = 0;

} // end <anonymous namespace>

GLuint compileShader(const char *shaderSource, GLenum stage)
{
	GLuint obj = gl::CreateShader(stage);
	const char *shaderSources[1] = { shaderSource };
	gl::ShaderSource(obj, 1, shaderSources, NULL);
	gl::CompileShader(obj);

	GLint status = gl::TRUE_;
	GLint logsize = 0;

	gl::GetShaderiv(obj, gl::COMPILE_STATUS, &status);
	gl::GetShaderiv(obj, gl::INFO_LOG_LENGTH, &logsize);
	if (status != gl::TRUE_) {
		ERROR << "Compile error:";
		if (logsize != 0) {
			char *logbuf = new char[logsize];
			gl::GetShaderInfoLog(obj, logsize, &logsize, logbuf);
			ERROR << logbuf;
			delete[] logbuf;
			gl::DeleteShader(obj);
		}
		else {
			ERROR << "<no log>";
		}
		throw std::runtime_error("shader compilation failed");
	}

	return obj;
}

// creates a shader program from vertex and fragment shader source files
/*GLuint createProgram(const char *vertexShaderSource, const char *fragmentShaderSource)
{
	GLuint vs_obj = compileShader(vertexShaderSource, gl::VERTEX_SHADER);
	GLuint fs_obj = compileShader(fragmentShaderSource, gl::FRAGMENT_SHADER);
	GLuint program_obj = gl::CreateProgram();
	gl::AttachShader(program_obj, vs_obj);
	gl::AttachShader(program_obj, fs_obj);
	linkProgram(program_obj);
	// once the program is linked, no need to keep the shader objects
	gl::DetachShader(program_obj, vs_obj);
	gl::DetachShader(program_obj, fs_obj);
	gl::DeleteShader(vs_obj);
	gl::DeleteShader(fs_obj);
	return program_obj;
}*/

GLuint compileShader(
	const char *source,
	const char *include_path,
	GLenum stage,
	util::array_ref<ShaderKeyword> keywords)
{
	auto hash = hashKeywords(keywords);
	namespace fs = boost::filesystem;
	std::istringstream sourceIn(source);
	auto pp = glslPreprocess(
		include_path,
		sourceIn,
		stage,
		keywords
		);
	// TODO error handling
	return compileShader(pp.c_str(), stage);
}

namespace
{
	void linkProgram(GLuint program)
	{
		GLint status = gl::TRUE_;
		GLint logsize = 0;

		gl::LinkProgram(program);
		gl::GetProgramiv(program, gl::LINK_STATUS, &status);
		gl::GetProgramiv(program, gl::INFO_LOG_LENGTH, &logsize);
		if (status != gl::TRUE_) {
			ERROR << "Link error:";
			if (logsize != 0) {
				char *logbuf = new char[logsize];
				gl::GetProgramInfoLog(program, logsize, &logsize, logbuf);
				ERROR << logbuf;
				delete[] logbuf;
			}
			else {
				ERROR << "<no log>";
			}
			throw std::runtime_error("link failed");
		}
	}
}

GLuint compileProgram(GLuint vs, GLuint gs, GLuint ps)
{
	auto program = gl::CreateProgram();
	assert(vs && ps);
	gl::AttachShader(program, vs);
	gl::AttachShader(program, ps);
	if (gs)
		gl::AttachShader(program, gs);
	linkProgram(program);
	gl::DetachShader(program, vs);
	gl::DetachShader(program, ps);
	if (gs) {
		gl::DetachShader(program, gs);
	}
	return program;
}

//=============================================================================
// TODO where should this be?
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
