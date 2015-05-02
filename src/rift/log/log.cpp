#include <log.hpp>
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Logging
{
	namespace
	{
		const char *getSeverityString(Severity severity)
		{
			switch (severity) {
			case Severity::Error:
				return "ERROR";
			case Severity::Warning:
				return "WARNING";
			case Severity::Info:
				return "INFO";
			}
			return "???";
		}

		//
		// Lines that should be displayed as a debug HUD
		std::vector<std::string> debugScreen;
	}

	//
	// output message 
	void outputMessage(Message const &message)
	{
		std::stringstream fmt_msg;
		fmt_msg << '[' << getSeverityString(message.severity) << "] ";
		fmt_msg << message.message.str().c_str() << '\n';
		auto str = fmt_msg.str();
		std::clog << str;
		// on windows, send it to the debug output
#ifdef _WIN32
		OutputDebugStringA(str.c_str());
#endif
	}

	void screenMessage(std::string message)
	{
		debugScreen.push_back(std::move(message));
	}

	std::vector<std::string> clearScreenMessages()
	{
		auto r = std::move(debugScreen);
		debugScreen.clear();
		return std::move(r);
	}

}