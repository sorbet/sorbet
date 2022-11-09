load("//tools:clang.bzl", "clang_tool")  # todo: this should be decoupled and use the library toolchain, not the compiler one
load(":cli_test.bzl", "cli_tests")

clang_tool("llvm-symbolizer")

cli_tests(
    "cli",
    glob(["*/*.sh"]),
)
