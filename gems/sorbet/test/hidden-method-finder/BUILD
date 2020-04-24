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
        "@ruby_2_4//:ruby",
        "@ruby_2_6//:ruby",
    ],
    tags = ["manual"],
    deps = [
        ":logging",
        "@bazel_tools//tools/bash/runfiles",
    ],
)

hidden_methods_tests(
    ruby = "ruby_2_4",
    tests = [
        "simple",
        "thorough",
    ],
)

hidden_methods_tests(
    ruby = "ruby_2_6",
    tests = [
        "simple",
        "thorough",
    ],
)

test_suite(
    name = "hidden-method-finder",
    tags = ["manual"],
    tests = [
        ":hidden-methods-ruby_2_4",
        ":hidden-methods-ruby_2_6",
    ],
)
