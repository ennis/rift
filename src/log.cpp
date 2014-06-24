#include "log.hpp"

#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#endif

Logger::~Logger()
{
	outFileStream.close();
}

//
// set log file
void Logger::init(const char *logFile)
{
	// build file name
	std::stringstream fileName;
	auto now = std::chrono::system_clock::now();
	fileName << logFile << '.' << now.time_since_epoch().count() << ".txt";
	// TODO append?
	outFileStream.open(fileName.str(), std::ios_base::out);
}

//
// send message 
void Logger::sendMessage(LogMessage const &message)
{
	// output severity string
	std::stringstream fmt_msg;
	fmt_msg << '(' << getSeverityString(message.severity) << ") (" << message.function << ") ";
	fmt_msg << message.message.str().c_str() << '\n';

	auto str = fmt_msg.str();
	outFileStream << str;
	outFileStream.flush();

	// also send it to the standard output
	std::clog << str;

	// on windows, send it to the debug output
#ifdef _WIN32
	OutputDebugStringA(str.c_str());
#endif
}

const char *Logger::getSeverityString(LogSeverity severity)
{
	switch (severity) {
	case LS_Error:
		return "ERROR";
	case LS_Warning:
		return "WARNING";
	case LS_Info:
		return "INFO";
	case LS_Debug:
		return "DEBUG";
	case LS_Trace:
		return "TRACE";
	default:
		return "UNSPECIFIED";
	}
}

//
// Logger instance
static Logger sLogger;

void logSendMessage(LogMessage const &message)
{
	sLogger.sendMessage(message);
}

void logInit(const char *logFile)
{
	sLogger.init(logFile);
}
