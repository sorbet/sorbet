load(":hidden_methods.bzl", "hidden_methods_tests")

sh_library(
    name = "logging",
    srcs = ["logging.sh"],
)

sh_binary(
    name = "hidden_methods_bazel",
    srcs = ["hidden_methods_bazel.sh"],
    data = [
        "//main:sorbet",
        "@sorbet_ruby_2_6//:ruby",
        "@sorbet_ruby_2_7//:ruby",
        "@sorbet_ruby_3_0//:ruby",
    ],
    deps = [
        ":logging",
        "@bazel_tools//tools/bash/runfiles",
    ],
)

hidden_methods_tests(
    ruby = "sorbet_ruby_2_6",
    tests = [
        "simple",
        "thorough",
    ],
)

hidden_methods_tests(
    ruby = "sorbet_ruby_2_7",
    tests = [
        "simple",
        "thorough",
    ],
)

hidden_methods_tests(
    ruby = "sorbet_ruby_3_0",
    tests = [
        "simple",
        "thorough",
    ],
)

test_suite(
    name = "hidden-method-finder",
    tests = [
        ":hidden-methods-sorbet_ruby_2_6",
        ":hidden-methods-sorbet_ruby_2_7",
        ":hidden-methods-sorbet_ruby_3_0",
    ],
)
