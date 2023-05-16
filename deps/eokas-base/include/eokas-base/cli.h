
#ifndef  _EOKAS_BASE_CLI_H_
#define  _EOKAS_BASE_CLI_H_

#include "./header.h"
#include "./string.h"
#include <optional>

_BeginNamespace(eokas::cli)

struct Option
{
	String name = "";
	String info = "";
	StringValue value = StringValue::falseValue;

	String toString() const;
};

struct Command
{
	using Func = std::function<void(const Command& cmd)>;

	String name;
	String info;
	std::map <String, Option> options = {};
	Func func;
	
	std::map <String, Command> subCommands = {};
	
	Command();
	Command(const String& name, const String& info = "");
	
	Command& option(const String& name, const String& info, const StringValue& defaultValue);
	Command& action(const Func& func);
	
	Command& subCommand(const String& name, const String& info);
	
	StringValue fetchValue(const String& shortName) const;
	std::optional<Option> fetchOption(const String& shortName) const;
	std::optional<Command> fetchCommand(const String& shortName) const;
	
	String toString() const;
	
	void exec(int argc, char** argv);
};

_EndNamespace(eokas::cli)

#endif//_EOKAS_BASE_CLI_H_
