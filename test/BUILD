test_suite(
    name = "test",
    tests = [
        "test_corpus",
    ],
)

load(":compiler_tests.bzl", "compiler_tests")
load("@com_stripe_ruby_typer//tools:clang.bzl", "clang_tool")  # todo: this should be decoupled and use the library toolchain, not the compiler one

clang_tool("llvm-diff")  # depended on by compiler_tests

compiler_tests(
    "test_corpus",
    glob([
        "testdata/compiler/**/*.rb",
    ]),
)

compiler_tests(
    "ruby_benchmark",
    glob([
        "testdata/ruby_benchmark/**/*.rb",
    ]),
)

sh_library(
    name = "logging",
    srcs = ["logging.sh"],
)

sh_library(
    name = "hermetic_tar",
    srcs = ["hermetic_tar.sh"],
)

sh_binary(
    name = "generate_out_file",
    srcs = ["generate_out_file.sh"],
    data = [
        "//run:runtimeoverrides",
        "@sorbet_ruby//:ruby",
    ],
    deps = [
        ":logging",
        "@bazel_tools//tools/bash/runfiles",
    ],
)

sh_binary(
    name = "build_extension",
    srcs = ["build_extension.sh"],
    data = ["//main:sorbet"],
    deps = [
        ":hermetic_tar",
        ":logging",
        "@bazel_tools//tools/bash/runfiles",
    ],
)
