#include "IncludePreprocessor.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <cassert>

namespace
{
	struct PpIncludeFile
	{
		// line in current source
		int lineNumber;
		// current source path
		std::string sourcePath;
		// Parent context
		PpIncludeFile *parentFile;
		int includeDepth;
	};

	struct PpContext
	{
		int addFilePath(const std::string& path) {
			filePathsById.push_back(path);
			return filePathsById.size() - 1;
		}

		// parsed GLSL version
		int glslVersion;
		std::vector<std::string> filePathsById;
		bool fatalError = false;
		bool error = false;
	};

	std::locale cloc("C");

	template <typename Fn>
	void SkipChars(const std::string &line, int &pos, Fn pred)
	{
		// skip whitespace
		while (pred(line[pos], cloc) && pos < line.size()) { pos++; }
	}

	bool IsIncludePathCharacter(char ch, const std::locale &loc)
	{
		return ch != '>';
	}

	std::string GetNextWord(const std::string &line, int &pos)
	{
		// skip whitespace
		SkipChars(line, pos, std::isspace<char>);
		int beg = pos;
		// get word
		SkipChars(line, pos, std::isalnum<char>);
		return line.substr(beg, pos - beg);
	}

	int ParseGlslVersion(const std::string &line, int &pos)
	{
		int end = line.size();
		// skip whitespace
		SkipChars(line, pos, std::isspace<char>);
		// version number
		int beg = pos;
		SkipChars(line, pos, std::isdigit<char>);
		auto version = line.substr(beg, pos - beg);
		// skip whitespace
		SkipChars(line, pos, std::isspace<char>);
		// must reach end of line
		if (pos != end) return 0;
		return std::stoi(version);
	}

	std::string ParseIncludePath(const std::string &line, int &pos)
	{
		int end = line.size();
		// skip whitespace
		SkipChars(line, pos, std::isspace<char>);
		if (pos == end || line[pos++] != '<') return "";
		int beg = pos;
		SkipChars(line, pos, IsIncludePathCharacter);
		auto path = line.substr(beg, pos - beg);
		// closing '<'
		if (pos == end || line[pos++] != '>') return "";
		// skip whitespace
		SkipChars(line, pos, std::isspace<char>);
		// must reach end of line
		if (pos != end) return "";
		return path;
	}

	const unsigned MaxIncludeDepth = 24;
}

void DumpIncludeStack(std::ostream& os, PpIncludeFile* file)
{
	using namespace std;
	PpIncludeFile* cur = file;
	while (cur != nullptr)
	{
		os << "\tin " << cur->sourcePath << " (" << cur->lineNumber << ')' << endl;
		cur = cur->parentFile;
	}
}

void ProcessIncludesImpl(
	std::istream& streamIn, const std::string& sourcePath, int sourceId, 
	const std::vector<std::string>& includePaths, std::ostream& outSource, 
	std::ostream &outInfoLog, PpContext& context, PpIncludeFile* parentFile = nullptr)
{
	using namespace std;
	namespace fs = std::experimental::filesystem;
	PpIncludeFile file;
	file.lineNumber = 1;
	file.parentFile = parentFile;
	file.sourcePath = string(sourcePath);
	file.includeDepth = parentFile ? (parentFile->includeDepth+1) : 0;
	string line;
	for (; getline(streamIn, line); ++file.lineNumber)
	{
		// if the first character of the line is a '#', then it is a preprocessing directive
		if (line[0] != '#') {
			outSource << line << endl;
			continue;
		}
		int pos = 1;
		string directive = GetNextWord(line, pos);
		if (directive == "version") {
			context.glslVersion = ParseGlslVersion(line, pos);
			continue;
		}
		if (directive != "pragma") {
			outSource << line << endl;
			continue;
		}
		string action = GetNextWord(line, pos);
		if (action == "include") 
		{
			auto include = ParseIncludePath(line, pos);
			if (include.size() == 0) {
				outInfoLog << "ERROR(" << file.lineNumber << ") Parse error in #pragma include" << endl;
				DumpIncludeStack(cerr, &file);
				context.error = true;
				outSource << line << endl;
				continue;
			}

			// look for include file in source path
			fs::path curSourcePath(sourcePath);
			auto incFilePath = curSourcePath.parent_path();
			incFilePath /= include;
			if (!exists(incFilePath)) {
				// does not exist, look in all include dirs
				bool found = false;
				for (const auto& incPath : includePaths)
				{
					incFilePath = incPath;
					incFilePath /= include;
					if (exists(incFilePath)) {
						found = true;
						break;
					}
				}
				if (!found) {
					outInfoLog << "FATAL(" << file.lineNumber << ") Include file not found: " << include << endl;
					DumpIncludeStack(cerr, &file);
					context.error = true;
					context.fatalError = true;
					return;
				}
			}
			// Check if the maximum include depth is reached
			if (file.includeDepth >= MaxIncludeDepth)
			{
				outInfoLog << "FATAL(" << file.lineNumber << ") Maximum include depth reached (circular include?)" << endl;
				DumpIncludeStack(cerr, &file);
				context.error = true;
				context.fatalError = true;
				return;
			}

			// insert include file in output stream
			auto incFilePathStr = incFilePath.string();
			ifstream includeFileIn(incFilePathStr);
			int incSourceId = context.addFilePath(incFilePathStr);
			outSource << "//====== INCLUDE FILE " << incFilePathStr << " FROM " << sourcePath << endl;
			outSource << "#line " << 1 << " " << incSourceId << endl;
			ProcessIncludesImpl(includeFileIn, incFilePathStr.c_str(), 
				incSourceId, includePaths, outSource, outInfoLog, context, &file);
			// Stop processing in case of fatal error
			if (context.fatalError) return;
			outSource << "//====== END OF INCLUDE FILE" << endl;
			outSource << "#line " << file.lineNumber + 1 << " " << sourceId << endl;
		}
		else 
		{
			// unrecognized pragma
			outSource << line << endl;
			continue;
		}
	}
}

bool ProcessIncludes(const std::string& sourcePath, 
	const std::vector<std::string>& includePaths, 
	std::ostream& outSource, std::ostream& outInfoLog)
{
	PpContext context;
	context.glslVersion = 110;
	std::ifstream sourceIn(sourcePath);
	int sourceId = context.addFilePath(sourcePath);
	ProcessIncludesImpl(sourceIn, sourcePath, sourceId, 
		includePaths, outSource, outInfoLog, context);
	return !context.error;
}
