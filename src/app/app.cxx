#include "app.h"
#include "./parser.h"
#include "./coder.h"


using namespace eokas;

#include <stdio.h>
#include <iostream>

static void eokas_main(coder_t& coder, const String& fileName, const String& cmd);

static void about(void);

static void help(void);

static void bad_command(const char* command);

int main(int argc, char** argv) {
    cli::Command program(argv[0]);

    coder_t coder;

    program.action([&](const cli::Command& cmd) -> void {
        about();
    });

    program.subCommand("help", "")
        .action([&](const cli::Command& cmd) -> void {
            help();
        });

    program.subCommand("compile", "")
        .option("--file,-f", "", "")
        .action([&](const cli::Command& cmd) -> void {
            auto file = cmd.fetchValue("--file").string();
            if (file.isEmpty())
                throw std::invalid_argument("The argument 'file' is empty.");

            printf("=> Source file: %s\n", file.cstr());

            eokas_main(coder, file, cmd.name);
        });

    program.subCommand("run", "")
        .option("--file,-f", "", "")
        .action([&](const cli::Command& cmd) -> void {
            auto file = cmd.fetchValue("--file").string();
            if (file.isEmpty())
                throw std::invalid_argument("The argument 'file' is empty.");
            if (!File::exists(file))
                throw std::invalid_argument(
                        String::format("The source file '%s' is not found.", file.cstr()).cstr());

            printf("=> Source file: %s\n", file.cstr());

            eokas_main(coder, file, cmd.name);
        });

    try {
        program.exec(argc, argv);
        return 0;
    }
    catch (const std::exception& e) {
        printf("\033[31mERROR: %s\033[0m", e.what());
        return -1;
    }
}

static void eokas_main(coder_t& coder, const String& fileName, const String& cmd) {
    FileStream in(fileName, "rb");
    if (!in.open())
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
    ast_node_module_t* node = parser.parse(source.cstr());
    if (node == nullptr) {
        const String& error = parser.error();
        printf("ERROR: %s\n", error.cstr());
        return;
    }

    omis_module_t* mod = coder.encode(node);
    if (mod == nullptr) {
        return;
    }

    FileStream out(String::format("%s.ll", fileName.cstr()), "w+");
    if (!out.open())
        return;

    printf("=> Encode to IR:\n");
    printf("------------------------------------------\n");
    printf(coder.dump(mod).cstr());
    printf("------------------------------------------\n");
    if(cmd == "run")
        coder.jit(mod);
    else
        coder.aot(mod);
    printf("------------------------------------------\n");
    out.close();
}

static void about(void) {
    printf("eokas %s\n", _EOKAS_VERSION);
}

static void help(void) {
    printf(
        "\n-?, -help\n"
        "\tPrint command line help message.\n"

        "\nfileName [-c] [-e] [-t]\n"
        "\tComple or Execute a file, show exec-time.\n"
   );
}

static void bad_command(const char* command) {
    printf(
        "The command '%s' is undefined in eokas. "
        "You can use the command %s to get the help infomation.\n", command, "'eokas -?' or 'eokas -help'"
   );
}
