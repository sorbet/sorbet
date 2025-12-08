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

            # Additional failing Prism tests - various location and desugaring differences
            "testdata/cfg/block_return_bug.rb",
            "testdata/compiler/block_iseq_names.rb",
            "testdata/compiler/default_with_escape.rb",
            "testdata/compiler/exceptions/raise_across_interop__*.rb",
            "testdata/compiler/exceptions/raise_across_interop.rb",
            "testdata/compiler/exceptions/rescue_in_block.rb",
            "testdata/compiler/intrinsics/array_compact.rb",
            "testdata/compiler/intrinsics/array_compact_bang.rb",
            "testdata/compiler/intrinsics/array_reject_bang.rb",
            "testdata/compiler/intrinsics/hash_merge.rb",
            "testdata/compiler/intrinsics/hash_merge_bang.rb",
            "testdata/compiler/regression/finalizer__*.rb",
            "testdata/compiler/regression/finalizer.rb",
            "testdata/compiler/string_interpolate_space.rb",
            "testdata/compiler/t_enum_absurd.rb",
            "testdata/core/fuzz_unparsable.rb",
            "testdata/desugar/csend.rb",
            "testdata/desugar/fuzz_block_pass.rb",
            "testdata/desugar/fuzz_break_do_end.rb",
            "testdata/desugar/numbered_parameters.rb",
            "testdata/desugar/pattern_matching_const.rb",
            "testdata/desugar/pattern_matching_pins.rb",
            "testdata/desugar/splat.rb",
            "testdata/infer/exhaustiveness.rb",
            "testdata/infer/fuzz_uninitialized_vars.rb",
            "testdata/infer/generics/option.rb",
            "testdata/infer/heredoc_loc.rb",
            "testdata/infer/isa_bug.rb",
            "testdata/infer/kwsplat_nil.rb",
            "testdata/infer/missing_method_name.rb",
            "testdata/infer/no_when_t_forwarder.rb",
            "testdata/infer/numbered_parameters.rb",
            "testdata/infer/or_and_conditional_branch.rb",
            "testdata/infer/singleton_case_exhaustiveness.rb",
            "testdata/infer/singleton_enum_case.rb",
            "testdata/infer/t_class_any.rb",
            "testdata/infer/t_proc_metatype.rb",
            "testdata/infer/tricky_pinning.rb",
            "testdata/infer/untyped.rb",
            "testdata/lsp/code_actions/convert_to_singleton_method/unary.rb",
            "testdata/lsp/code_actions/extract_variable_multiple/case.rb",
            "testdata/lsp/code_actions/extract_variable_multiple/rescue.rb",
            "testdata/lsp/code_actions/extract_variable_multiple/scope.rb",
            "testdata/lsp/code_actions/extract_variable_single/invalid.rb",
            "testdata/lsp/code_actions/extract_variable_single/other.rb",
            "testdata/lsp/code_actions/extract_variable_single/rescue.rb",
            "testdata/lsp/completion/error_recovery_binop_unop.rb",
            "testdata/lsp/completion/if_empty.rb",
            "testdata/lsp/completion/instance_var.rb",
            "testdata/lsp/completion/instance_var_strict.rb",
            "testdata/lsp/completion/method_kwargs.rb",
            "testdata/lsp/completion/method_kwargs_extra.rb",
            "testdata/lsp/completion/missing_assign_rhs.rb",
            "testdata/lsp/completion/operators.rb",
            "testdata/lsp/completion/setter.rb",
            "testdata/lsp/completion/snippet_before_variable.rb",
            "testdata/lsp/completion/superclass.rb",
            "testdata/lsp/completion/symbol_proc.rb",
            "testdata/lsp/completion/untyped_parse_errors.rb",
            "testdata/lsp/completion/untyped_receiver.rb",
            "testdata/lsp/hover_it_param.rb",
            "testdata/lsp/hover_methods_inside_block.rb",
            "testdata/lsp/when_eqeqeq.rb",
            "testdata/namer/duplicate_field_error.rb",
            "testdata/parser/bad_argument_crash.rb",
            "testdata/parser/chained_sends_lots.rb",
            "testdata/parser/compare_overload_parse_error.rb",
            "testdata/parser/crash_case_empty_var.rb",
            "testdata/parser/curly_braces_block_pass.rb",
            "testdata/parser/fuzz_ivar.rb",
            "testdata/parser/heredoc_chomp.rb",
            "testdata/parser/heredoc_squiggle_dedent.rb",
            "testdata/parser/heredoc_xstring.rb",
            "testdata/parser/it_param_invalid_syntax.rb",
            "testdata/parser/it_param_with_explicit_args.rb",
            "testdata/parser/kwargs_missing_comma_unsupported.rb",
            "testdata/parser/long_string.rb",
            "testdata/rbi/proc.rb",
            "testdata/rbs/assertions_heredoc_modified.rb",
            "testdata/resolver/bad_sealed_class_absurd__*.rb",
            "testdata/resolver/it_param_rspec_method.rb",
            "testdata/resolver/sig_misc.rb",
            "testdata/resolver/empty_sealed.rb",
            "testdata/resolver/it_param_nested_blocks.rb",
            "testdata/resolver/it_param_proc_lambda.rb",
            "testdata/resolver/sealed_class.rb",
            "testdata/resolver/sealed_concrete_parent_class.rb",
            "testdata/resolver/sealed_module.rb",
            "testdata/rewriter/initializer_no_arg_with_local.rb",
            "testdata/rewriter/t_enum_exhaustiveness.rb",
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
    glob(
        [
            "prism_regression/**/*.rb",
            "prism_regression/**/*.exp",
        ],
        exclude = [
            # Location differences in error recovery - Prism provides tighter/different
            # location spans for ErrorNode compared to Whitequark:
            # - `x =` (missing RHS) has different Assign/ErrorNode locations
            "prism_regression/error_recovery/assign.rb",
            # - `1 +` (missing operand) has different Send/ErrorNode locations
            "prism_regression/invalid/missing_node.rb",
            # - `/foo` (unterminated regex) - Prism includes content up to EOF,
            #   Whitequark produces empty regex
            "prism_regression/invalid/unterminated_regex.rb",

            # Numbered block parameter location differs: Prism uses last occurrence
            # of `_1`, Whitequark uses first occurrence
            "prism_regression/multiple_numbered_params.rb",

            # Prism desugared tree doesn't match legacy desugared tree (desugar differences)
            "prism_regression/assign_to_index.rb",
            "prism_regression/call_implicit_kw_args.rb",
            "prism_regression/for_loop.rb",
            "prism_regression/literal_hash.rb",
            "prism_regression/multi_write.rb",
        ],
    ),
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
