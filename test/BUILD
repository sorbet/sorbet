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
    ]),
    linkstatic = 1,
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast/desugar",
        "//cfg",
        "//namer",
        "//infer",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "hello-test",
    size = "small",
    srcs = ["hello-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = 1,
    visibility = ["//tools:__pkg__"],
    deps = [
        "//common",
        "//parser",
        "@cxxopts//:cxxopts",
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
