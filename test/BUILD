cc_binary(
    name = "test_corpus_sharded",
    testonly = 1,
    srcs = [
        "LSPTest.cc",
        "LSPTest.h",
        "expectations.h",
        "lsp_test_helpers.cc",
        "lsp_test_helpers.h",
        "position_assertions.cc",
        "position_assertions.h",
        "test_corpus.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast/desugar",
        "//ast/treemap",
        "//cfg",
        "//cfg/builder",
        "//core",
        "//dsl",
        "//infer",
        "//main/autogen",
        "//main/lsp",
        "//namer",
        "//payload:test_bootstrap",
        "//resolver",
        "@com_google_googletest//:gtest",
        "@cxxopts",
    ],
)

cc_test(
    name = "hello-test",
    size = "small",
    srcs = ["hello-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast",
        "//ast/treemap",
        "//common",
        "//core",
        "//core/serialize",
        "//parser",
        "//payload:test_bootstrap",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@cxxopts",
    ],
)

cc_test(
    name = "error-check-test",
    size = "small",
    srcs = ["error-check-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast",
        "//ast/desugar",
        "//common",
        "//core",
        "//parser",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "autocorrect-test",
    size = "small",
    srcs = ["autocorrect-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//common",
        "//core",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
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
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
)

cc_binary(
    name = "backtrace-test-error",
    srcs = ["backtrace-test-error.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = ["//common"],
)

load(":pipeline_test.bzl", "pipeline_tests")

test_corpus_tests = pipeline_tests(
    glob([
        "testdata/**/*.rb",
        "testdata/**/*.exp",
        "testdata/**/*.exp.svg",
    ]),
    "PosTests",
    filter = "PosTests",
)

test_corpus_lsp_tests_fastpath = pipeline_tests(
    glob([
        "testdata/**/*.rb",
        "testdata/**/*.exp",
        "testdata/**/*.exp.svg",
    ]),
    "LSPFastPathTests",
    filter = "LSPTests",
)

test_corpus_lsp_tests_slowpath = pipeline_tests(
    glob([
        "testdata/**/*.rb",
        "testdata/**/*.exp",
        "testdata/**/*.exp.svg",
    ]),
    "LSPSlowPathTests",
    extra_args = ["--lsp-disable-fastpath"],
    filter = "LSPTests",
    tags = ["manual"],
)

test_suite(
    name = "test_corpus",
    tests = test_corpus_tests,
)

test_suite(
    name = "test_corpus_lsp_fastpath",
    tests = test_corpus_lsp_tests_fastpath,
)

test_suite(
    name = "test_corpus_lsp_slowpath",
    tags = ["manual"],
    tests = test_corpus_lsp_tests_slowpath,
)

test_suite(
    name = "test",
    tests = ["test_corpus"],
)
