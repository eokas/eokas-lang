
#ifndef  _EOKAS_BASE_LOGGER_H_
#define  _EOKAS_BASE_LOGGER_H_

#include "header.h"
#include "string.h"
#include "signal.h"
#include <fstream>

_BeginNamespace(eokas)

enum class LogLevel
{
	Verbose,
	Notice,
	Warning,
	Error
};

class Logger
{
public:
	static void push(const String& name);
	static void pop();
	static Logger* log();
	static Logger* log(const String& name);

public:
	Logger();
	~Logger();
	bool open(const String& name);
	void close();
	void info(const char* fmt, ...);
	void warning(const char* fmt, ...);
	void error(const char* fmt, ...);
	void message(LogLevel level, const String& message);

public:
	struct LogSignalMessage
	{
		LogLevel level;
		String message;
	};
	Signal<LogSignalMessage&> callback;

private:
	std::ofstream mFile;
};

#define _log_push(name) Logger::push(name)
#define _log_pop() Logger::pop()
#define _log_info(fmt, ...) Logger::log()->info(fmt,  __VA_ARGS__)
#define _log_warning(fmt, ...) Logger::log()->warning(fmt, __VA_ARGS__)
#define _log_error(fmt, ...) Logger::log()->error(fmt, __VA_ARGS__)
#define _log_info_to(name, fmt, ...) Logger::log(name)->info(fmt, __VA_ARGS__)
#define _log_warning_to(name, fmt, ...) Logger::log(name)->warning(fmt, __VA_ARGS__)
#define _log_error_to(name, fmt, ...) Logger::log(name)->error(fmt, __VA_ARGS__)

_EndNamespace(eokas)

#endif//_EOKAS_BASE_LOGGER_H_
