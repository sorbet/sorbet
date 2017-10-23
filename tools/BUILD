load("//tools:clang.bzl", "clang_tool")
load("@compdb//:aspects.bzl", "compilation_database")

clang_tool("clang-format")

compilation_database(
    name = "compdb",
    testonly = True,
    exec_root_marker = True,
    targets = [
        "//test:hello-test",
        "//main:hello-world",
        "//main:hello-greet",
        "//parser:parser_test",
        "//parser:parse_ast",
        "//ast/desugar:desugar_ast",
        "//ast/desugar:desugar_test",
        "//test:test_corpus",
        "//namer:namer_print",
    ],
)
