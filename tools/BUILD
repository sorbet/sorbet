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
        "//main:sorbet_llvm",
        "//plugin_injector:plugin_injector",
        "//llvm/payload:generate_load_payload",
        "//llvm/linker:linker",
        "//llvm/payload:payload",
        # END compile_commands targets
    ],
)
