#include "app.h"
#include "parser/parser.h"
#include "llvm/engine.h"

using namespace eokas;

#include <stdio.h>
#include <iostream>

static void eokas_main(const String& fileName, bool(*)(ast_module_t*));

static void about(void);

static void help(void);

static void bad_command(const char* command);

int main(int argc, char** argv)
{
	cli::Command cli;
	
	cli.command("about", "About eokas.", [&](auto& cmd)->int
	{
		about();
		return 0;
	});
	cli.command("help", "Show eokas command-line messages.", [&](auto& cmd)->int
	{
		help();
		return 0;
	});
	cli.command("compile", "Compile eokas source code.", [&](const cli::Command& cmd)->int
	{
		const auto& fileOption = cmd.options.at("--file,-f");
		String fileName = fileOption.value;
		printf("=> Source file: %s\n", fileName.cstr());
		try
		{
			eokas_main(fileName, llvm_aot);
			return 0;
		}
		catch (std::exception& e)
		{
			printf("ERROR: %s \n", e.what());
			return 1;
		}
	});
	cli.command("run", "Run eokas source just in time", [&](const cli::Command& cmd)->int
	{
		const auto& fileOption = cmd.options.at("--file,-f");
		String fileName = fileOption.value;
		printf("=> Source file: %s\n", fileName.cstr());
		try
		{
			eokas_main(fileName, llvm_jit);
			return 0;
		}
		catch (std::exception& e)
		{
			printf("ERROR: %s \n", e.what());
			return 1;
		}
	});
	
	cli.set(argv[0], "eokas-lang", [&](auto& cmd)->int
	{
		help();
	});
	
	return cli.exec(argc, argv);
}

static void eokas_main(const String& fileName, bool(*proc)(ast_module_t* m))
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
	proc(m);
	printf("------------------------------------------\n");
	out.close();
}

static void about()
{
	printf("eokas %s\n", _EOKAS_VERSION);
}

static void help()
{
	printf("\n-?, -help\n"
		   "\tPrint command line help message.\n"
	
		   "\nfileName [-c] [-e] [-t]\n"
		   "\tComple or Execute a file, show exec-time.\n");
}
