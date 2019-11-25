load("//tools:clang.bzl", "clang_tool")
load("@compdb//:aspects.bzl", "compilation_database")

clang_tool("clang-format")

clang_tool("clang-tidy")

compilation_database(
    name = "compdb",
    testonly = True,
    targets = [
        # BEGIN compile_commands targets
        "//main:sorbet",
        "//plugin_injector:plugin_injector",
        "//compiler/Payload:pseudo_lib",
        "//compiler/Payload:generate_load_payload",
        "//compiler/ObjectFileEmitter:ObjectFileEmitter",
        "//compiler/Passes:Passes",
        "//compiler/Linker:Linker",
        "//compiler/IREmitter:IREmitter",
        "//compiler/Payload:Payload",
        "//compiler/Errors:Errors",
        "//compiler/DefinitionRewriter:DefinitionRewriter",
        "//compiler/Names:Names",
        "//compiler/Core:Core",
        # END compile_commands targets
    ],
)
