load("//tools:clang.bzl", "clang_tool")
load("@compdb//:aspects.bzl", "compilation_database")

clang_tool("clang-format")

clang_tool("clang-tidy")

compilation_database(
    name = "compdb",
    testonly = True,
    targets = [
        # BEGIN compile_commands targets
        "//test/llvm:kaleidoscope",
        "//main:sorbet",
        "//plugin_injector:plugin_injector",
        "//compiler/ObjectFileEmitter:ObjectFileEmitter",
        "//compiler/LLVMIREmitter:LLVMIREmitter",
        "//compiler/IRHelpers:pseudo_lib",
        "//compiler/IRHelpers:generate_load_payload",
        "//compiler/IRHelpers:IRHelpers",
        "//compiler/Errors:Errors",
        "//compiler/DefinitionRewriter:DefinitionRewriter",
        "//compiler/Names:Names",
        # END compile_commands targets
    ],
)
