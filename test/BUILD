cc_binary(
    name = "test_corpus_sharded",
    testonly = 1,
    srcs = [
        "test_corpus.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast/desugar",
        "//ast/treemap",
        "//cfg",
        "//cfg/builder",
        "//core",
        "//core/serialize",
        "//dsl",
        "//infer",
        "//main/autogen",
        "//main/errorqueue",
        "//namer",
        "//payload/binary:some",
        "//resolver",
        "@cxxopts",
        "@gtest",
    ],
)

cc_test(
    name = "hello-test",
    size = "small",
    srcs = ["hello-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast",
        "//ast/treemap",
        "//common",
        "//core",
        "//core/serialize",
        "//parser",
        "//payload/binary:some",
        "@cxxopts",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "error-check-test",
    size = "small",
    srcs = ["error-check-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast",
        "//ast/desugar",
        "//common",
        "//core",
        "//parser",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "autocorrect-test",
    size = "small",
    srcs = ["autocorrect-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//common",
        "//core",
        "//main/errorqueue",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

# Passes with --config=dbg but I'm not smart enough to figure out how to make it
# only run when in that config
# sh_test(
#     name = "backtrace-test",
#     size = "small",
#     srcs = ["backtrace-test.sh"],
#     data = ["backtrace-test-raise"],
# )

cc_binary(
    name = "backtrace-test-raise",
    srcs = ["backtrace-test-raise.cc"],
    copts = ["--std=c++14"],
    visibility = ["//tools:__pkg__"],
)

cc_binary(
    name = "backtrace-test-error",
    srcs = ["backtrace-test-error.cc"],
    copts = ["--std=c++14"],
    visibility = ["//tools:__pkg__"],
    deps = ["//common"],
)

load(":pipeline_test.bzl", "pipeline_tests")

corpus_tests = pipeline_tests(glob([
    "testdata/**/*.rb",
    "testdata/**/*.exp",
    "testdata/**/*.exp.svg",
]))

test_suite(
    name = "test_corpus",
    tests = corpus_tests,
)
