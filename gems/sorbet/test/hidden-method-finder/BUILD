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
        "@gems//bundler",
        "@gems//bundler:bundle",
        "@ruby_2_4_3//:ruby",
        "@ruby_2_6_3//:ruby",
    ],
    tags = ["manual"],
    deps = [
        ":logging",
        "@bazel_tools//tools/bash/runfiles",
    ],
)

hidden_methods_tests(
    ruby = "ruby_2_4_3",
    tests = ["simple", "thorough"],
)

hidden_methods_tests(
    ruby = "ruby_2_6_3",
    tests = ["simple", "thorough"],
)

test_suite(
    name = "hidden-methods-finder",
    tags = ["manual"],
    tests = [
        ":hidden-methods-ruby_2_4_3",
        ":hidden-methods-ruby_2_6_3",
    ],
)

test_suite(
    name = "hidden-methods-update",
    tags = ["manual"],
    tests = [
        ":hidden-methods-update-ruby_2_4_3",
        ":hidden-methods-update-ruby_2_6_3",
    ],
)
