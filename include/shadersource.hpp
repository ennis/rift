#ifndef SHADERSOURCE_HPP
#define SHADERSOURCE_HPP

#include <common.hpp>
#include <resource.hpp>
#include <vector>
#include <sstream>
#include <string>
#include <memory>

//=============================================================================
enum class ShaderSourceType
{
	Vertex,
	Fragment,
	Geometry
};

//=============================================================================
// preprocessed shader source code
class ShaderSource
{
public:
	friend class EffectCompiler;

	ShaderSource() = default;
	ShaderSource(const char *fileName, ShaderSourceType shaderSourceType);
	virtual ~ShaderSource();

	void loadFromFile(const char *fileName, ShaderSourceType shaderSourceType);
	// used internally
	void setShaderNodeID(unsigned int id);
	unsigned int getShaderNodeID() const;

	// preprocessor
	void preprocess(std::ostream &fileOut, const char *includeDir, const char *defines);

private:
	void preprocessPrivate(
		std::istream &fileIn, 
		std::ostream &os, 
		const char *includeDir, 
		int includeDepth,
		const char *defines);

	const char *mSourcePath = nullptr;
	std::string mSource;
	unsigned int mID;
	ShaderSourceType mSourceType;
};

//=============================================================================
std::string loadShaderSource(const char *path);

#endif