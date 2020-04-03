load("//tools:clang.bzl", "clang_tool")
load("@compdb//:aspects.bzl", "compilation_database")

clang_tool("clang-format")

clang_tool("clang-tidy")

compilation_database(
    name = "compdb",
    testonly = True,
    targets = [
        # BEGIN compile_commands targets
        "//third_party/xxd:xxd",
        "//main:sorbet",
        "//plugin_injector:plugin_injector",
        "//compiler/ObjectFileEmitter:ObjectFileEmitter",
        "//compiler/Passes:Passes",
        "//compiler/Linker:Linker",
        "//compiler/IREmitter/Payload:pseudo_lib",
        "//compiler/IREmitter/Payload:generate_load_payload",
        "//compiler/IREmitter:IREmitter",
        "//compiler/Names:Names",
        "//compiler/IREmitter/Payload:Payload",
        "//compiler/Core:Core",
        "//compiler/Errors:Errors",
        # END compile_commands targets
    ],
)
