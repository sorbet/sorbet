# Load the macro that creates a test for every file in the prism_regression folder
load("//test:prism_location_test.bzl", "prism_location_test_suite")
load("//tools:clang.bzl", "clang_tool")  # todo: this should be decoupled and use the library toolchain, not the compiler one
load(":pipeline_test.bzl", "pipeline_tests")

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

# Run the macro
prism_location_test_suite(
    name = "prism_location_tests",
    srcs = glob(
        ["prism_regression/**/*.rb"],
        exclude = [
            "prism_regression/error_recovery/assign.rb",
            "prism_regression/lambda.rb",
            "prism_regression/call_kw_nil_args.rb",
            "prism_regression/call_block_param_and_forwarding.rb",
            "prism_regression/constants_invalid.rb",
            "prism_regression/invalid/unterminated_regex.rb",

            # Sorbet's parser includes the last newline in the `<ErrorNode>`, but Prism doesn't.
            "prism_regression/invalid/missing_node.rb",
        ],
    ),
)

# Create a test suite to run all the location tests
test_suite(
    name = "prism_location_tests",
    tags = ["prism_location_test"],
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
        "//test/helpers",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

cc_test(
    name = "pkg-condensation-test",
    size = "small",
    srcs = ["pkg_condensation_test.cc"],
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
        "//test/helpers",
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

cc_test(
    name = "cancelable-indexer-test",
    size = "small",
    srcs = ["cancelable-indexer-test.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast",
        "//common",
        "//core",
        "//main/options",
        "//main/pipeline",
        "//payload",
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

pipeline_tests(
    "test_corpus",
    glob(
        [
            "testdata/**/*.rb",
            "testdata/**/*.exp",
        ],
        exclude = [
            # Specific test that is modified for Prism, contains minor differences in the exp file
            "testdata/rbs/assertions_heredoc_modified.rb",
        ],
    ),
    "PosTests",
)

pipeline_tests(
    "test_corpus_prism",
    glob(
        [
            # Replace [parser] with other phases to test Prism at that level
            # Phases: https://github.com/Shopify/sorbet/blob/prism/docs/internals.md#phases
            "testdata/**/*.rb",
            "testdata/**/*.exp",
        ],
        exclude = [
            # Tests having to do with tree differences in invalid Ruby code; will address later
            "testdata/parser/error_recovery/**",
            "testdata/parser/crash_block_pass_suggestion.rb",
            "testdata/parser/invalid_fatal.rb",
            "testdata/parser/invalid_syntax_error.rb",
            "testdata/parser/kwargs_missing_comma.rb",
            "testdata/parser/kwnil_errors.rb",  # See https://github.com/Shopify/sorbet/issues/393
            "testdata/parser/misc.rb",
            "testdata/parser/offset0.rb",
            "testdata/parser/ruby_25.rb",
            "testdata/parser/kwargs.rb",
            "testdata/lsp/completion/bad_arguments.rb",
            "testdata/lsp/completion/bad_list_elems.rb",
            "testdata/lsp/completion/case_1.rb",
            "testdata/lsp/completion/case_2.rb",
            "testdata/lsp/completion/eof.rb",
            "testdata/lsp/completion/missing_const_name.rb",
            "testdata/lsp/completion/missing_fun.rb",
            "testdata/lsp/completion/self_receiver.rb",
            "testdata/lsp/duplicate_kwarg.rb",
            "testdata/namer/fuzz_repeated_kwarg.rb",
            "testdata/deviations/keyword_method_names.rb",
            "testdata/rewriter/minitest_empty_test_each.rb",
            "testdata/infer/crash_after_parse_errors.rb",
            "testdata/parser/invalid_def_block_param_after_forward.rb",
            "testdata/parser/invalid_def_forward_after_block_param.rb",

            # Sorbets' parser incorrectly accepts invalid syntax
            "testdata/desugar/pattern_matching_hash.rb",  # See https://github.com/Shopify/sorbet/issues/362

            # Minor differences in the parse tree lead to different exp files
            "testdata/rbs/assertions_heredoc.rb",

            # Whitequark and Prism parsers disagree on inclusion of parentheses in the location.
            # This is surfaced with PM_AND_NODE and PM_OR_NODE desugaring a right hand side enclosed in parentheses.
            # See comment in PM_PARENTHESES_NODE in Translator.cc.
            #   "Override the begin node location to be the parentheses location instead of the statements location"
            # However, undoing that override leads to issues with nested parentheses handling.
            "testdata/infer/control_flow/complex_implication_1.rb",

            # Prism doesn't have enough context to reject f(&b, 1), and attempts to treat it equivalently to f(1, &b)
            "testdata/desugar/bad_block_pass.rb",
        ],
    ),
    "PrismPosTests",
    parser = "prism",
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
    "prism_regression_corpus",
    glob([
        "prism_regression/**/*.rb",
        "prism_regression/**/*.exp",
    ]),
    "PrismPosTests",
    parser = "prism",
)

test_suite(
    name = "test",
    tests = ["test_corpus"],
)

test_suite(
    name = "test_prism",
    tests = ["test_corpus_prism"],
)

test_suite(
    name = "whitequark_parser_tests",
    tests = ["whitequark_parser_corpus"],
)

test_suite(
    name = "prism_regression",
    tests = ["prism_regression_corpus"],
)
