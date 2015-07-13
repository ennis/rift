#ifndef INCLUDE_PREPROCESSOR_HPP
#define INCLUDE_PREPROCESSOR_HPP

#include <string>
#include <vector>
#include <iosfwd>

bool ProcessIncludes(const std::string& sourcePath, 
	const std::vector<std::string>& includePaths, 
	std::ostream& outSource, std::ostream& outInfoLog);

#endif
