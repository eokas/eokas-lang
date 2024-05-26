#include "app.h"
#include "./parser.h"
#include "./coder.h"

using namespace eokas;

#include <stdio.h>

static void eokas_main(coder_t& coder, const String& fileName, const String& cmd);
static void about(void);
static void help(void);
static void bad_command(const char* command);
static String read_text_file(String& filePath);

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

static void eokas_main(coder_t& coder, const String& file, const String& cmd) {
    std::map<String, ast_node_module_t*> modules;

    std::vector<String> filesToParse;
    filesToParse.push_back(file);

    for(size_t index = 0; index < filesToParse.size(); index++) {
        String& filePath = filesToParse[index];

        // parsed already
        if(modules.find(filePath) != modules.end()) {
            continue;
        }

        String source = read_text_file(filePath);
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

        if(!node->imports.empty()) {
            String fileHome = File::basePath(filePath);
            for(auto& item : node->imports) {
                String targetPath = item.second->target;

                // If the target is a relative path, it refers to a
                // location relative to the current file.
                if(targetPath.startsWith(".")) {
                    targetPath = File::combinePath(fileHome, targetPath);
                    targetPath = File::absolutePath(targetPath);
                }

                auto iter = std::find(filesToParse.begin(), filesToParse.end(), targetPath);
                if(iter == filesToParse.end()) {
                    filesToParse.push_back(targetPath);
                }
            }
        }

        modules.insert(std::make_pair(filePath, node));
    }

    omis_module_t* mainModule = nullptr;
    // NOTE: index >= 0 is always true because of size_t is unsigned.
    for(size_t index = filesToParse.size() - 1; index < filesToParse.size(); index--) {
        const String& filePath = filesToParse[index];
        ast_node_module_t* node = modules[filePath];

        omis_module_t* mod = coder.encode(node);
        if (mod == nullptr) {
            return;
        }

        if(filePath == file) {
            mainModule = mod;
        }
    }

    if(mainModule == nullptr) {
        return;
    }
    FileStream out(String::format("%s.ll", file.cstr()), "w+");
    if (!out.open())
        return;

    printf("=> Encode to IR:\n");
    printf("------------------------------------------\n");
    printf("%s", coder.dump(mainModule).cstr());
    printf("------------------------------------------\n");
    if(cmd == "run")
        coder.jit(mainModule);
    else
        coder.aot(mainModule);
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

static String read_text_file(String& filePath) {
    FileStream in(filePath, "rb");
    if (!in.open())
        return "";

    size_t size = in.size();
    MemoryBuffer buffer(size);
    in.read(buffer.data(), buffer.size());
    in.close();

    String content((const char*)buffer.data(), buffer.size());

    return content;
}
