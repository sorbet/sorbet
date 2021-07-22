#include "main/lsp/tools/generate_lsp_messages.h"
#include "main/lsp/tools/make_lsp_types.h"

using namespace std;

const string JSONArrayType::arrayVar = "a";

bool writeFile(char *path, fmt::memory_buffer &buffer) {
    ofstream stream(path, ios::trunc);
    if (!stream.good()) {
        cerr << "unable to open " << path << '\n';
        return false;
    }
    stream << to_string(buffer);
    return true;
}

int main(int argc, char **argv) {
    if (argc < 5) {
        cerr << "Error: Expected 4 input files\n";
        return 1;
    }

    vector<std::shared_ptr<JSONClassType>> enumTypes;
    vector<std::shared_ptr<JSONObjectType>> classTypes;
    makeLSPTypes(enumTypes, classTypes);

    // Emit enums.
    {
        fmt::memory_buffer enumHeaderBuffer;
        fmt::memory_buffer enumClassFileBuffer;

        fmt::format_to(std::back_inserter(enumClassFileBuffer), "#include \"main/lsp/json_types.h\"\n");
        fmt::format_to(std::back_inserter(enumClassFileBuffer), "#include \"main/lsp/lsp_messages_gen_helpers.h\"\n");
        fmt::format_to(std::back_inserter(enumClassFileBuffer), "namespace sorbet::realmain::lsp {{\n");

        // Emits enums before class definitions themselves.
        for (auto &enumType : enumTypes) {
            enumType->emit(enumHeaderBuffer, enumClassFileBuffer);
        }
        fmt::format_to(std::back_inserter(enumClassFileBuffer), "}}\n");

        if (!writeFile(argv[3], enumHeaderBuffer)) {
            return 1;
        }

        if (!writeFile(argv[4], enumClassFileBuffer)) {
            return 1;
        }
    }

    // Emit classes.
    {
        fmt::memory_buffer headerBuffer;
        fmt::memory_buffer classFileBuffer;

        fmt::format_to(std::back_inserter(classFileBuffer), "#include \"main/lsp/json_types.h\"\n");
        fmt::format_to(std::back_inserter(classFileBuffer), "#include \"main/lsp/lsp_messages_gen_helpers.h\"\n");
        fmt::format_to(std::back_inserter(classFileBuffer), "namespace sorbet::realmain::lsp {{\n");

        for (auto &classType : classTypes) {
            classType->emit(headerBuffer, classFileBuffer);
        }
        fmt::format_to(std::back_inserter(classFileBuffer), "}}\n");

        // Output buffers to files.
        if (!writeFile(argv[1], headerBuffer)) {
            return 1;
        }

        if (!writeFile(argv[2], classFileBuffer)) {
            return 1;
        }
    }

    return 0;
}
