
#include "app.h"
#include "parser.h"
using namespace eokas;

#include <stdio.h>

static void eokas_main(const char* file);
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
    else if(!command.startsWith("-"))
    {
        String file = command;
        eokas_main(file.cstr());
    }
    else
    {
        bad_command(command.cstr());
    }

    return 0;
}

static void eokas_main(const char* fileName)
{
    FileStream stream(fileName, "rb");
    if (!stream.open())
        return;
    
    size_t size = stream.size();
    MemoryBuffer buffer(size);
    stream.read(buffer.data(), buffer.size());
    stream.close();
    
    String str((const char*)buffer.data(), buffer.size());
    
    parser_t parser;
    ast_module_t* t = parser.parse(str.cstr());
    if (t == nullptr)
    {
        String error = parser.error();
        return;
    }
    
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
