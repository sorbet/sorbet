load("//tools:clang.bzl", "clang_tool")
load("@compdb//:aspects.bzl", "compilation_database")

clang_tool("clang-format")

compilation_database(
    name = "compdb",
    testonly = True,
    exec_root_marker = True,
    targets = [
        "//test:hello-test",
        "//parser:parser_test",
        "//ast/desugar:desugar_test",
        "//test:test_corpus",
        "//main:ruby-typer"
    ],
)
