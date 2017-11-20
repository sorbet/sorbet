load("//tools:clang.bzl", "clang_tool")
load("@compdb//:aspects.bzl", "compilation_database")

clang_tool("clang-format")

compilation_database(
    name = "compdb",
    testonly = True,
    exec_root_marker = True,
    targets = [
        # BEGIN compile_commands targets
        "//test:test_corpus",
        "//test:hello-test",
        "//test:error-check-test",
        "//test:backtrace-test-raise",
        "//test:backtrace-test-error",
        "//payload/text:generate_payload",
        "//payload/binary:gen_state_payload",
        "//parser:parser_test",
        "//parser:generate_ast",
        "//namer:namer_test",
        "//main:stdlib_test",
        "//main:ruby-typer-orig",
        "//payload/binary:empty",
        "//main:ruby-typer",
        "//payload/binary:binary",
        "//main:realmain",
        "//payload/text:text",
        "//payload/binary:some",
        "//infer:infer_test",
        "//namer:namer",
        "//infer:infer",
        "//core/serialize:serialize_test",
        "//core/serialize:serialize",
        "//core:generate_names",
        "//core:core_test",
        "//cfg:cfg",
        "//ast/desugar:desugar_test",
        "//ast/desugar:desugar",
        "//parser:parser",
        "//ast:ast",
        "//core:core",
        "//common:common",
        # END compile_commands targets
    ],
)
