test_suite(
    name = "test",
    tests = [
        "test_corpus",
    ],
)

load(":compiler_tests.bzl", "compiler_tests")

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

sh_binary(
    name = "generate_out_file",
    srcs = ["generate_out_file.sh"],
    data = [
        "//run:runtimeoverrides",
        "@ruby_2_6_3//:ruby",
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
        ":logging",
        "@bazel_tools//tools/bash/runfiles",
    ],
)
