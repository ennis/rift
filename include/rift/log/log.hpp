#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <sstream>
#include <fstream>

enum LogCategory
{
	LC_Game,
	LC_Renderer,
	LC_Unspecified
};

enum LogSeverity
{
	LS_Error,
	LS_Warning,
	LS_Info,
	LS_Debug,
	LS_Trace,
	LS_Unspecified
};

class LogMessage;

void logSendMessage(LogMessage const &message);
void logInit(const char *logFile);

class Logger
{
public:
	~Logger();
	void init(const char *logFile);
	void sendMessage(LogMessage const &message);

private:
	static const char *getSeverityString(LogSeverity severity);

#ifdef CONFIG_LOG_TO_FILE
	std::ofstream outFileStream;
#endif
};

class LogMessage
{
public:
	friend class Logger;

	LogMessage(LogSeverity severity_, const char *function_, const char *file_, int line_) : category(LC_Unspecified), function(function_), severity(severity_), file(file_), line(line_)
	{}

	~LogMessage() 
	{
		logSendMessage(*this);
	}

	template <typename T>
	LogMessage &operator<<(T const& t) 
	{
		message << t;
		return *this;
	}

private:
	std::stringstream message;
	const char *function;
	const char *file;
	int line;
	LogCategory category;
	LogSeverity severity;
};

#ifdef _MSC_VER 
// MSVC isn't YET fully c++11 compliant.
#define __func__ __FUNCTION__
#endif

#undef ERROR
#undef WARNING
#define LOG LogMessage(LS_Debug, __func__, __FILE__, __LINE__)
#define ERROR LogMessage(LS_Error, __func__, __FILE__, __LINE__)
#define WARNING LogMessage(LS_Warning, __func__, __FILE__, __LINE__)

#endif