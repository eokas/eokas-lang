
#include "app.h"
#include "parser/parser.h"
#include "llvm/engine.h"

using namespace eokas;

#include <stdio.h>
#include <iostream>

static void eokas_main(const String& file);

static void about(void);

static void help(void);

static void bad_command(const char* command);

int main(int argc, char** argv)
{
	Args args(argc, argv);
	if(args.count()<2)
	{
		about();
		return 0;
	}
	
	String command = args.get(1);
	if(command == "-?" || command == "-help")
	{
		help();
	}
	else if(command.startsWith("-c"))
	{
		String file = args.get(2);
		printf("=> Source file: %s\n", file.cstr());
		try
		{
			eokas_main(file);
		}
		catch (std::exception& e)
		{
			printf("ERROR: %s \n", e.what());
		}
	}
	else
	{
		bad_command(command.cstr());
	}
	
	return 0;
}

static void eokas_main(const String& fileName)
{
	FileStream in(fileName, "rb");
	if(!in.open())
		return;
	
	size_t size = in.size();
	MemoryBuffer buffer(size);
	in.read(buffer.data(), buffer.size());
	in.close();
	
	String source((const char*) buffer.data(), buffer.size());
	printf("=> Source code:\n");
	printf("------------------------------------------\n");
	printf("%s\n", source.replace("%", "%%").cstr());
	printf("------------------------------------------\n");
	
	parser_t parser;
	ast_module_t* m = parser.parse(source.cstr());
	printf("=> Module AST: %X\n", m);
	if(m == nullptr)
	{
		const String& error = parser.error();
		printf("ERROR: %s\n", error.cstr());
		return;
	}
	
	FileStream out(String::format("%s.ll", fileName.cstr()), "w+");
	if(!out.open())
		return;
	
	printf("=> Encode to IR:\n");
	printf("------------------------------------------\n");
	llvm_jit(m);
	printf("------------------------------------------\n");
	out.close();
	
	// todo:
}

static void about(void)
{
	printf(
		"eokas %s\n",
		_EOKAS_VERSION
	);
}

static void help(void)
{
	printf(
		"\n-?, -help\n"
		"\tPrint command line help message.\n"
		
		"\nfileName [-c] [-e] [-t]\n"
		"\tComple or Execute a file, show exec-time.\n"
	);
}

static void bad_command(const char* command)
{
	printf(
		"The command '%s' is undefined in eokas. "
		"You can use the command %s to get the help infomation.\n",
		command,
		"'eokas -?' or 'eokas -help'"
	);
}
