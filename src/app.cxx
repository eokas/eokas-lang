
#include "app.h"
#include "parser.h"
#include "coder.h"
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
    if (args.count() < 2)
    {
        about();
        return 0;
    }

    String command = args.get(1);
    if (command == "-?" || command == "-help")
    {
        help();
    }
    else if (command.startsWith("-c"))
    {
        String file = args.get(2);
        printf("file: %s\n", file.cstr());
        eokas_main(file);
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
    if (!in.open())
        return;

    size_t size = in.size();
    MemoryBuffer buffer(size);
    in.read(buffer.data(), buffer.size());
    in.close();

    String str((const char*)buffer.data(), buffer.size());
    printf("%s\n", str.cstr());

    parser_t parser;
    ast_module_t* m = parser.parse(str.cstr());
    printf("module: %x\n", m);
    if (m == nullptr)
    {
        const String& error = parser.error();
        printf("ERROR: %s\n", error.cstr());
        return;
    }

    FileStream out(String::format("%s.cxx", fileName.cstr()), "w+");
    if(!out.open())
        return;

    printf("begin encode\n");
    coder_t coder(out);
    coder.encode_module(m);
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
