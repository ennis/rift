#include <shadersource.hpp>
#include <regex>
#include <cstring>
#include <fstream>
#include <log.hpp>

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

//=============================================================================
std::vector<std::string> split(const std::string& input, const std::regex& regex) {
	// passing -1 as the submatch index parameter performs splitting
	const std::sregex_token_iterator
		first(input.begin(), input.end(), regex, -1),
		last;
	return{ first, last };
}


//=============================================================================
ShaderSource::~ShaderSource()
{
}

//=============================================================================
ShaderSource::ShaderSource(const char *fileName, ShaderSourceType shaderType) : mSourcePath(fileName)
{
	loadFromFile(fileName, shaderType);
}

//=============================================================================
void ShaderSource::loadFromFile(const char *fileName, ShaderSourceType shaderType)
{
	// open input file
	LOG << "Loading shader node " << fileName;
	mSourcePath = fileName;
	mSource = loadShaderSource(fileName);
	mSourceType = shaderType;
}

//=============================================================================
// preprocessor
void ShaderSource::preprocess(
	std::ostream &fileOut,
	const char *includeDir, 
	const char *defines)
{
	std::istringstream is(mSource);
	preprocessPrivate(is, fileOut, includeDir, 0, defines);
}

//=============================================================================
static bool tryInputFile(
	std::ifstream &fileIn,
	std::string const &fileName,
	std::ios::openmode openMode)
{
	fileIn.clear();
	fileIn.close();
	fileIn.open(fileName.c_str(), openMode);
	if (fileIn.fail()) {
		return false;
	}
	return true;
}

//=============================================================================
// preprocessor
void ShaderSource::preprocessPrivate(
	std::istream &fileIn, 
	std::ostream &fileOut, 
	const char *includeDir, 
	int includeDepth, 
	const char *defines)
{
	static const int kMaxIncludeDepth = 20;
	if (includeDepth > kMaxIncludeDepth) {
		ERROR << "Maximum include depth reached (circular include?)";
		return;
	}
	std::string line;
	int ln = 1;
	const char *err_invalidDirective = "Invalid directive at line ";
	const char *err_invalidPragma = "Unrecognized pragma at line ";
	while (std::getline(fileIn, line)) {
		if (line[0] == '#') {
			auto tokens = split(line, std::regex("\\s+"));
			if (tokens[0] == "#pragma") {
				if (tokens.size() < 2) {
					WARNING << err_invalidPragma << ln;
				}
				else {
					auto &&pragma = tokens[1];
					if (pragma == "include") {
						// include directive
						auto &&path = tokens[2];
						if (path[0] != '<' || *(path.crbegin()) != '>') {
							ERROR << "Bad file path format at line " << ln;
						}
						else {
							// strip '<' and '>'
							path.erase(path.begin());
							path.pop_back();
							// try file path relative to source path first
							std::string sp(mSourcePath);
							auto l = sp.find_last_of('/');
							if (l != std::string::npos) {
								sp.erase(sp.begin() + l + 1, sp.end());
							}
							sp += path;
							LOG << "Trying " << sp.c_str();
							std::ifstream incFileIn(sp.c_str(), std::ios::binary);
							if (incFileIn.fail()) {
								// failed, look in supplied include directory
								incFileIn.clear();
								auto &&path2 = std::string(includeDir) + "/" + path;
								LOG << "Trying " << path2.c_str();
								incFileIn.open(path2.c_str(), std::ios::binary);
								// cannot fail now
								assert(!incFileIn.fail());
							}
							preprocessPrivate(incFileIn, fileOut, includeDir, includeDepth + 1, defines);
						}
					}
				}
			} else {
				fileOut << line << '\n';
			}
		} else {
			fileOut << line << '\n';;
		}
		++ln;
	}
}

//=============================================================================
unsigned int ShaderSource::getShaderNodeID() const
{
	return mID;
}

//=============================================================================
void ShaderSource::setShaderNodeID(unsigned int id)
{
	mID = id;
}
