load("//tools:clang.bzl", "clang_tool")  # todo: this should be decoupled and use the library toolchain, not the compiler one

clang_tool("llvm-symbolizer")

cc_binary(
    name = "pipeline_test_runner",
    testonly = 1,
    srcs = [
        "pipeline_test_runner.cc",
    ],
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
        "//class_flatten",
        "//core",
        "//definition_validator",
        "//infer",
        "//local_vars",
        "//main/autogen",
        "//main/lsp",
        "//main/minimize",
        "//main/pipeline",
        "//main/pipeline/semantic_extension:none",
        "//namer",
        "//packager",
        "//payload",
        "//resolver",
        "//rewriter",
        "//test/helpers",
        "@cxxopts",
        "@doctest//doctest",
        "@doctest//doctest:custom_main",
    ],
)

cc_binary(
    name = "lsp_test_runner",
    testonly = 1,
    srcs = [
        "lsp_test_runner.cc",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//common",
        "//main/lsp",
        "//payload",
        "//test/helpers",
        "@cxxopts",
        "@doctest//doctest",
        "@doctest//doctest:custom_main",
    ],
)

cc_binary(
    name = "parser_test_runner",
    testonly = 1,
    srcs = [
        "parser_test_runner.cc",
    ],
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
        "//class_flatten",
        "//core",
        "//definition_validator",
        "//infer",
        "//local_vars",
        "//main/autogen",
        "//main/pipeline/semantic_extension:none",
        "//namer",
        "//payload",
        "//resolver",
        "//rewriter",
        "//test/helpers",
        "@cxxopts",
        "@doctest//doctest",
        "@doctest//doctest:custom_main",
    ],
)

cc_binary(
    name = "print_document_symbols",
    testonly = 1,
    srcs = [
        "print_document_symbols.cc",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//main/lsp",
        "//payload",
    ],
)

sh_test(
    name = "print_document_symbols_builds",
    srcs = ["print_document_symbols_builds.sh"],
    data = [
        ":print_document_symbols",
    ],
)

cc_test(
    name = "hello-test",
    size = "small",
    srcs = ["hello-test.cc"],
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
        "//payload",
        "@cxxopts",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

cc_test(
    name = "pkg-autocorrects-test",
    size = "small",
    srcs = ["pkg_autocorrects_test.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast",
        "//ast/desugar",
        "//common",
        "//common/concurrency",
        "//core",
        "//local_vars",
        "//packager",
        "//parser",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

cc_test(
    name = "error-check-test",
    size = "small",
    srcs = ["error-check-test.cc"],
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
        "@doctest//doctest",
        "@doctest//doctest:main",
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

load(":pipeline_test.bzl", "pipeline_tests", "single_package_rbi_test")

filegroup(
    name = "rbi_gen_files",
    srcs = glob(["testdata/packager/rbi_gen_files/**/*.rb"]),
)

single_package_rbi_test(
    name = "end_to_end_rbi_test",
    rb_files = ["//test:rbi_gen_files"],
)

pipeline_tests(
    "test_corpus",
    glob(
        [
            "testdata/**/*.rb",
            "testdata/**/*.exp",
        ],
    ),
    "PosTests",
)

pipeline_tests(
    "whitequark_parser_corpus",
    glob([
        "whitequark/**/*.rb",
        "whitequark/**/*.exp",
    ]),
    "WhitequarkParserTests",
)

pipeline_tests(
    "test_corpus_lsp",
    glob(
        [
            "testdata/**/*.rb",
            "testdata/**/*.rbi",
            "testdata/**/*.exp",
        ],
    ),
    "LSPTests",
    extra_files = ["testdata/lsp/rubyfmt-stub/rubyfmt"],
)

pipeline_tests(
    "test_corpus_prism",
    glob([
        "prism_regression/**/*.rb",
    ]),
    "PosTests",
    parser = "prism",
)

test_suite(
    name = "test",
    tests = ["test_corpus"],
)

test_suite(
    name = "whitequark_parser_tests",
    tests = ["whitequark_parser_corpus"],
)

test_suite(
    name = "prism",
    tests = ["test_corpus_prism"],
)

sh_binary(
    name = "single_package_runner",
    srcs = ["test_single_package_runner.sh"],
    data = [
        "single_package_runner_cc",
        "//main:sorbet",
    ],
    tags = ["manual"],
    deps = [
        "@bazel_tools//tools/bash/runfiles",
    ],
)

cc_binary(
    name = "single_package_runner_cc",
    srcs = ["rbi_gen_package_runner.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    tags = ["manual"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "//common",
        "@com_google_absl//absl/strings",
        "@rapidjson",
    ],
)
