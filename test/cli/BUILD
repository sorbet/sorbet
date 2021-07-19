load(":cli_test.bzl", "cli_tests")

cli_tests(
    name = "cli",
    tests = glob(["*/test.sh"]),
)

sh_binary(
    name = "run_test",
    srcs = ["run.sh"],
    data = ["//main:sorbet", "@sorbet_ruby_2_7//:ruby"],
    deps = ["@bazel_tools//tools/bash/runfiles"],
)
