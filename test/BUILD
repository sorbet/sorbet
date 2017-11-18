cc_test(
    name = "test_corpus",
    size = "small",
    srcs = [
        "test_corpus.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = glob([
        "testdata/**/*.rb",
        "testdata/**/*.rb.*.exp",
        "testdata/**/*.rb.*.exp.svg",
    ]),
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast/desugar",
        "//cfg",
        "//core",
        "//infer",
        "//namer",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)

filegroup(
    name = "gerald",
    srcs = [
        "testdata/first/gerald.rb",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "hello-test",
    size = "small",
    srcs = ["hello-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast",
        "//common",
        "//core",
        "//parser",
        "@cxxopts//:cxxopts",
        "@gtest//:gtest",
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
        "@gtest//:gtest",
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
)

cc_binary(
    name = "backtrace-test-error",
    srcs = ["backtrace-test-error.cc"],
    deps = ["//common"],
)

sh_test(
    name = "end-to-end-test",
    size = "small",
    srcs = ["end-to-end-test.sh"],
    data = [
        "end-to-end-test-input.rb",
        "end-to-end-test-output",
        "//main:ruby-typer",
    ],
)
