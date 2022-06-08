
#ifndef  _EOKAS_BASE_CLI_H_
#define  _EOKAS_BASE_CLI_H_

#include "header.h"
#include "string.h"

_BeginNamespace(eokas::cli)

struct Option
{
	String name = "";
	String info = "";
	StringValue value = StringValue::falseValue;

	void set(const String& name, const String& info, const StringValue& value);
	[[nodiscard]] String toString() const;
};

struct Command
{
	using Func = std::function<int(const Command& cmd)>;

	String name;
	String info;
	Func func;
	std::map <String, Option> options = {};
	std::map <String, Command> subCommands = {};
	
	void set(const String& name, const String& info, const Func& func);
	Option option(const String& name, const String& info, const StringValue& defaultValue);
	Command command(const String& name, const String& info, const Func& func);
	[[nodiscard]] String toString() const;
	int exec(int argc, char** argv);
};

_EndNamespace(eokas::cli)

#endif//_EOKAS_BASE_CLI_H_
