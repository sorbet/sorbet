#include "main/lsp/tools/generate_lsp_messages.h"
#include "main/lsp/tools/make_lsp_types.h"

using namespace std;

const string JSONArrayType::arrayVar = "a";

bool writeFile(const char *path, fmt::memory_buffer &buffer) {
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
        cerr << "Error: Expected at least 4 input files\n";
        return 1;
    }

    auto enumHeader = argv[1];
    auto msgHeader = argv[3];

    std::vector<const char *> enumSource{argv[2]};

    std::vector<const char *> msgSources;
    for (auto i = 4; i < argc; i++) {
        msgSources.push_back(argv[i]);
    }

    vector<std::shared_ptr<JSONClassType>> enumTypes;
    vector<std::shared_ptr<JSONObjectType>> classTypes;
    makeLSPTypes(enumTypes, classTypes);

    // Emit enums.
    {
        fmt::memory_buffer enumHeaderBuffer;
        std::vector<fmt::memory_buffer> enumClassFileBuffer(enumSource.size());

        for (auto &buffer : enumClassFileBuffer) {
            fmt::format_to(std::back_inserter(buffer), "#pragma GCC diagnostic push\n");
            fmt::format_to(std::back_inserter(buffer),
                           "#pragma GCC diagnostic ignored \"-Wunused-but-set-variable\"\n");
            fmt::format_to(std::back_inserter(buffer), "#include \"main/lsp/json_types.h\"\n");
            fmt::format_to(std::back_inserter(buffer), "#include \"main/lsp/lsp_messages_gen_helpers.h\"\n");
            fmt::format_to(std::back_inserter(buffer), "namespace sorbet::realmain::lsp {{\n");
        }

        // Emits enums before class definitions themselves.
        int i = 0;
        for (auto &enumType : enumTypes) {
            auto &buffer = enumClassFileBuffer[i++ % enumClassFileBuffer.size()];
            enumType->emit(enumHeaderBuffer, buffer);
        }

        for (auto &buffer : enumClassFileBuffer) {
            fmt::format_to(std::back_inserter(buffer), "}}\n");
            fmt::format_to(std::back_inserter(buffer), "#pragma GCC diagnostic pop\n");
        }

        if (!writeFile(enumHeader, enumHeaderBuffer)) {
            return 1;
        }

        for (auto i = 0; i < enumSource.size(); i++) {
            auto *source = enumSource[i];
            auto &buffer = enumClassFileBuffer[i];
            if (!writeFile(source, buffer)) {
                return 1;
            }
        }
    }

    // Emit classes.
    {
        fmt::memory_buffer headerBuffer;
        std::vector<fmt::memory_buffer> classFileBuffer(msgSources.size());

        for (auto &buffer : classFileBuffer) {
            fmt::format_to(std::back_inserter(buffer), "#pragma GCC diagnostic push\n");
            fmt::format_to(std::back_inserter(buffer),
                           "#pragma GCC diagnostic ignored \"-Wunused-but-set-variable\"\n");
            fmt::format_to(std::back_inserter(buffer), "#include \"main/lsp/json_types.h\"\n");
            fmt::format_to(std::back_inserter(buffer), "#include \"main/lsp/lsp_messages_gen_helpers.h\"\n");
            fmt::format_to(std::back_inserter(buffer), "namespace sorbet::realmain::lsp {{\n");
        }

        int i = 0;
        for (auto &classType : classTypes) {
            auto &buffer = classFileBuffer[i++ % classFileBuffer.size()];
            classType->emit(headerBuffer, buffer);
        }

        for (auto &buffer : classFileBuffer) {
            fmt::format_to(std::back_inserter(buffer), "}}\n");
            fmt::format_to(std::back_inserter(buffer), "#pragma GCC diagnostic pop\n");
        }

        // Output buffers to files.
        if (!writeFile(msgHeader, headerBuffer)) {
            return 1;
        }

        for (auto i = 0; i < msgSources.size(); i++) {
            auto *source = msgSources[i];
            auto &buffer = classFileBuffer[i];
            if (!writeFile(source, buffer)) {
                return 1;
            }
        }
    }

    return 0;
}
