#include <gl4/renderer.hpp>
#include <sstream>
#include <algorithm>
#include <vector>
#include <log.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iomanip>
#include <array_ref.hpp>

namespace gl4
{
	namespace {
		//=========================================================================
		//=========================================================================
		// GLSL
		//=========================================================================
		//=========================================================================
		GLuint glslCompileShader(const char *shaderSource, GLenum type)
		{
			GLuint obj = gl::CreateShader(type);
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

		void glslLinkProgram(GLuint program)
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

		// creates a shader program from vertex and fragment shader source files
		GLuint glslCreateProgram(const char *vertexShaderSource, const char *fragmentShaderSource)
		{
			GLuint vs_obj = glslCompileShader(vertexShaderSource, gl::VERTEX_SHADER);
			GLuint fs_obj = glslCompileShader(fragmentShaderSource, gl::FRAGMENT_SHADER);
			GLuint program_obj = gl::CreateProgram();
			gl::AttachShader(program_obj, vs_obj);
			gl::AttachShader(program_obj, fs_obj);
			glslLinkProgram(program_obj);
			// once the program is linked, no need to keep the shader objects
			gl::DetachShader(program_obj, vs_obj);
			gl::DetachShader(program_obj, fs_obj);
			gl::DeleteShader(vs_obj);
			gl::DeleteShader(fs_obj);
			return program_obj;
		}

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

		enum class GLShaderStage
		{
			Vertex,
			Fragment
		};

		struct GLSLKeyword
		{
			std::string define;
			std::string value;
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
			std::array_ref<GLSLKeyword> keywords,
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
			source << tmp.str();
			return source.str();
		}

		std::size_t hashKeywords(
			std::array_ref<GLSLKeyword> keywords)
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


		std::pair<std::string, std::string> preprocessGLSLEffect(
			const char *vs_source,
			const char *ps_source,
			const char *include_path,
			std::array_ref<GLSLKeyword> keywords
			)
		{
			auto hash = hashKeywords(keywords);
			//LOG << "Hash " << hash;
			/*auto &cs = mShaderCache[hash];
			if (cs != nullptr) {
			// already compiled
			return cs.get();
			}*/

			std::ostringstream shader_name;
			shader_name << '/'
				<< std::setw(16) << std::setfill('0') << std::hex << hash << std::dec << '/';
			// log keywords
			for (const auto &k : keywords) {
				shader_name << k.define;
				if (k.value.size()) {
					shader_name << '=' << k.value;
				}
				shader_name << '/';
			}
			LOG << "Compiling " << shader_name.str();

			std::string vs, fs;
			// combined source file
			std::istringstream vsIn(vs_source);
			// preprocess source file as a vertex shader source
			vs = glslPreprocess(
				include_path,
				vsIn,
				keywords,
				GLShaderStage::Vertex
				);
			// rewind stream and parse as a fragment shader source
			std::istringstream psIn(ps_source);
			fs = glslPreprocess(
				include_path,
				psIn,
				keywords,
				GLShaderStage::Fragment
				);

			// create new shader 
			// TODO error handling
			return std::make_pair(vs, fs);
		}

	} // end gl4::<anonymous namespace>


	//=============================================================================
	//=============================================================================
	// Renderer::createEffect
	//=============================================================================
	//=============================================================================
	Effect::Effect(
		const char *combinedShaderSource,
		const char *includePath,
		RasterizerDesc rasterizerState,
		DepthStencilDesc depthStencilState
		) :
		Effect(combinedShaderSource, combinedShaderSource, includePath, rasterizerState, depthStencilState)
	{
	}
	
	Effect::Effect(
		const char *vsSource,
		const char *psSource,
		const char *includePath,
		RasterizerDesc rasterizerState,
		DepthStencilDesc depthStencilState) 
	{
		cache_id = sCurrentEffectCacheID++;
		ds_state = depthStencilState;
		rs_state = rasterizerState;
		// preprocess source
		std::pair<std::string, std::string> sources = preprocessGLSLEffect(
			vsSource,
			psSource,
			includePath,
			std::array_ref < GLSLKeyword > {});
		program =
			util::unique_resource<GLuint, Effect::Deleter>(
			glslCreateProgram(
			sources.first.c_str(),
			sources.second.c_str()));
	}
}


//=============================================================================
// TODO where should this be?
std::string loadEffectSource(const char *fileName)
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