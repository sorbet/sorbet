#include "main/lsp/tools/generate_lsp_messages.h"
#include "main/lsp/tools/make_lsp_types.h"

using namespace std;

const string JSONArrayType::arrayVar = "a";

int main(int argc, char **argv) {
    fmt::memory_buffer headerBuffer;
    fmt::memory_buffer classFileBuffer;

    fmt::format_to(classFileBuffer, "#include \"main/lsp/json_types.h\"\n");
    fmt::format_to(classFileBuffer, "#include \"main/lsp/lsp_messages_gen_helpers.h\"\n");
    fmt::format_to(classFileBuffer, "namespace sorbet::realmain::lsp {{\n");

    vector<std::shared_ptr<JSONClassType>> enumTypes;
    vector<std::shared_ptr<JSONObjectType>> classTypes;
    makeLSPTypes(enumTypes, classTypes);

    // Emits enums before class forward decls before class definitions themselves.
    for (auto &enumType : enumTypes) {
        enumType->emit(headerBuffer, classFileBuffer);
    }
    for (auto &classType : classTypes) {
        classType->emitForwardDeclaration(headerBuffer);
    }
    for (auto &classType : classTypes) {
        classType->emit(headerBuffer, classFileBuffer);
    }
    fmt::format_to(classFileBuffer, "}}\n");

    // Output buffers to files.
    ofstream header(argv[1], ios::trunc);
    if (!header.good()) {
        cerr << "unable to open " << argv[1] << '\n';
        return 1;
    }
    header << to_string(headerBuffer);

    ofstream classfile(argv[2], ios::trunc);
    if (!classfile.good()) {
        cerr << "unable to open " << argv[2] << '\n';
        return 1;
    }
    classfile << to_string(classFileBuffer);
    return 0;
}
