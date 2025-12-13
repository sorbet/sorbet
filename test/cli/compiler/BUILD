load(":cli_test.bzl", "cli_tests")

cli_tests(
    name = "compiler",
    tags = ["compiler"],
    tests = glob(["*/test.sh"]),
)

sh_binary(
    name = "run_test",
    srcs = ["run.sh"],
    data = [
        "//compiler:sorbet",
        "@sorbet_ruby_2_7_for_compiler//:ruby",
    ],
    tags = ["compiler"],
    deps = ["@bazel_tools//tools/bash/runfiles"],
)
