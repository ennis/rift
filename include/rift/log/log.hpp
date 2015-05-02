#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

namespace Logging
{
	enum class Severity
	{
		Error,
		Warning,
		Info
	};

	struct Message;

	void outputMessage(Message const &message);
	void screenMessage(std::string message);
	std::vector<std::string> clearScreenMessages();

	struct Message
	{
	public:
		Message(Severity severity_, const char *function_, const char *file_, int line_) : function(function_), severity(severity_), file(file_), line(line_)
		{}

		~Message()
		{
			outputMessage(*this);
		}

		template <typename T>
		Message &operator<<(T const& t)
		{
			message << t;
			return *this;
		}

		std::stringstream message;
		const char *function;
		const char *file;
		int line;
		Severity severity;
	};
}

#ifdef _MSC_VER 
// MSVC isn't YET fully c++11 compliant.
#define __func__ __FUNCTION__
#endif

#undef ERROR
#undef WARNING
#define LOG Logging::Message(Logging::Severity::Info, __func__, __FILE__, __LINE__)
#define ERROR Logging::Message(Logging::Severity::Error, __func__, __FILE__, __LINE__)
#define WARNING Logging::Message(Logging::Severity::Warning, __func__, __FILE__, __LINE__)

#endif