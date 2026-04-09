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
        "//ast/desugar/prism",
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
        "//main/options",
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
    test_name_prefix = "PosTests",
    tests = glob(
        [
            "testdata/**/*.rb",
            "testdata/**/*.exp",
        ],
        exclude = [
            "testdata/rbs/**/*.rb",
            "testdata/rbs/**/*.exp",
        ],
    ),
)

pipeline_tests(
    "test_corpus_prism",
    disabled_tests = [
        # Prism allows a new line between `.` and `end` as a way to call a method called literally `#end`.
        # The legacy parser treats this as an incomplete call to `x.<method-name-missing>()`, which is much
        # nicer behaviour, which we eventually want to support with Prism as well.
        # https://github.com/Shopify/sorbet/issues/739
        "testdata/lsp/completion/missing_fun.rb",

        # Tests having to do with tree differences in invalid Ruby code; will address later
        "testdata/parser/error_recovery/assign.rb",
        "testdata/parser/error_recovery/begin_1.rb",
        "testdata/parser/error_recovery/block_do_1.rb",
        "testdata/parser/error_recovery/case_1.rb",
        "testdata/parser/error_recovery/case_2.rb",
        "testdata/parser/error_recovery/circular_argument_reference.rb",
        "testdata/parser/error_recovery/class_indent_1.rb",
        "testdata/parser/error_recovery/class_indent_2.rb",
        "testdata/parser/error_recovery/class_indent_3.rb",
        "testdata/parser/error_recovery/class_indent_4.rb",
        "testdata/parser/error_recovery/class_indent_5.rb",
        "testdata/parser/error_recovery/class_indent_6.rb",
        "testdata/parser/error_recovery/class_weird_newline_indent.rb",
        "testdata/parser/error_recovery/const_case.rb",
        "testdata/parser/error_recovery/constant_only_scope.rb",
        "testdata/parser/error_recovery/def_missing_end_1.rb",
        "testdata/parser/error_recovery/def_missing_end_2.rb",
        "testdata/parser/error_recovery/defn_indent_1.rb",
        "testdata/parser/error_recovery/defn_indent_2.rb",
        "testdata/parser/error_recovery/defn_indent_3.rb",
        "testdata/parser/error_recovery/defn_indent_4.rb",
        "testdata/parser/error_recovery/defn_indent_5.rb",
        "testdata/parser/error_recovery/defn_indent_6.rb",
        "testdata/parser/error_recovery/defs_indent_1.rb",
        "testdata/parser/error_recovery/defs_indent_2.rb",
        "testdata/parser/error_recovery/defs_indent_3.rb",
        "testdata/parser/error_recovery/defs_indent_4.rb",
        "testdata/parser/error_recovery/defs_indent_5.rb",
        "testdata/parser/error_recovery/defs_indent_6.rb",
        "testdata/parser/error_recovery/eof_1.rb",
        "testdata/parser/error_recovery/eof_2.rb",
        "testdata/parser/error_recovery/eof_3.rb",
        "testdata/parser/error_recovery/eof_4.rb",
        "testdata/parser/error_recovery/eof_5.rb",
        "testdata/parser/error_recovery/eof_6.rb",
        "testdata/parser/error_recovery/eof_7.rb",
        "testdata/parser/error_recovery/eof_9.rb",
        "testdata/parser/error_recovery/forward_args.rb",
        "testdata/parser/error_recovery/if_do_1.rb",
        "testdata/parser/error_recovery/if_do_2.rb",
        "testdata/parser/error_recovery/if_indent_1.rb",
        "testdata/parser/error_recovery/if_indent_2.rb",
        "testdata/parser/error_recovery/if_indent_3.rb",
        "testdata/parser/error_recovery/if_indent_4.rb",
        "testdata/parser/error_recovery/if_indent_5.rb",
        "testdata/parser/error_recovery/if_no_end.rb",
        "testdata/parser/error_recovery/lonely_def_01.rb",
        "testdata/parser/error_recovery/lonely_def_02.rb",
        "testdata/parser/error_recovery/lonely_def_03.rb",
        "testdata/parser/error_recovery/lonely_def_04.rb",
        "testdata/parser/error_recovery/lonely_def_05.rb",
        "testdata/parser/error_recovery/lonely_def_06.rb",
        "testdata/parser/error_recovery/lonely_def_07.rb",
        "testdata/parser/error_recovery/lonely_def_08.rb",
        "testdata/parser/error_recovery/lonely_def_09.rb",
        "testdata/parser/error_recovery/lonely_def_10.rb",
        "testdata/parser/error_recovery/lonely_def_11.rb",
        "testdata/parser/error_recovery/lonely_def_12.rb",
        "testdata/parser/error_recovery/lonely_def_13.rb",
        "testdata/parser/error_recovery/lonely_def_14.rb",
        "testdata/parser/error_recovery/long_line_indent.rb",
        "testdata/parser/error_recovery/long_prefix_indent.rb",
        "testdata/parser/error_recovery/missing_fun.rb",
        "testdata/parser/error_recovery/missing_operator.rb",
        "testdata/parser/error_recovery/module_indent_1.rb",
        "testdata/parser/error_recovery/module_indent_2.rb",
        "testdata/parser/error_recovery/module_indent_3.rb",
        "testdata/parser/error_recovery/module_indent_4.rb",
        "testdata/parser/error_recovery/module_indent_5.rb",
        "testdata/parser/error_recovery/module_indent_6.rb",
        "testdata/parser/error_recovery/multiple_stmts.rb",
        "testdata/parser/error_recovery/numparams.rb",
        "testdata/parser/error_recovery/numparams_crash_1.rb",
        "testdata/parser/error_recovery/numparams_crash_2.rb",
        "testdata/parser/error_recovery/numparams_crash_3.rb",
        "testdata/parser/error_recovery/numparams_crash_5.rb",
        "testdata/parser/error_recovery/pos_after_kw_1.rb",
        "testdata/parser/error_recovery/send_after_end.rb",
        "testdata/parser/error_recovery/unmatched_block_args_1.rb",
        "testdata/parser/error_recovery/unmatched_block_args_2.rb",
        "testdata/parser/error_recovery/unmatched_block_args_3.rb",
        "testdata/parser/error_recovery/unmatched_block_args_4.rb",
        "testdata/parser/error_recovery/unterminated_array.rb",
        "testdata/parser/error_recovery/unterminated_array_bad_1.rb",
        "testdata/parser/error_recovery/unterminated_array_bad_2.rb",
        "testdata/parser/crash_block_pass_suggestion.rb",
        "testdata/parser/invalid_fatal.rb",
        "testdata/parser/invalid_syntax_error.rb",
        "testdata/parser/kwargs_missing_comma.rb",
        "testdata/parser/kwnil_errors.rb",  # See https://github.com/Shopify/sorbet/issues/393
        "testdata/parser/offset0.rb",
        "testdata/parser/kwargs.rb",
        "testdata/lsp/completion/bad_arguments.rb",
        "testdata/lsp/completion/bad_list_elems.rb",
        "testdata/lsp/completion/case_1.rb",
        "testdata/lsp/completion/case_2.rb",
        "testdata/lsp/completion/eof.rb",
        "testdata/lsp/completion/missing_const_name.rb",
        "testdata/lsp/completion/self_receiver.rb",
        "testdata/lsp/completion/enum_case.rb",
        "testdata/lsp/completion/enum_case_crash.rb",
        "testdata/lsp/duplicate_kwarg.rb",
        "testdata/namer/fuzz_repeated_kwarg.rb",
        "testdata/deviations/keyword_method_names.rb",
        "testdata/rewriter/minitest_empty_test_each.rb",
        "testdata/parser/invalid_def_block_param_after_forward.rb",
        "testdata/parser/invalid_def_forward_after_block_param.rb",

        # Sorbets' parser incorrectly accepts invalid syntax
        "testdata/desugar/pattern_matching_hash.rb",  # See https://github.com/Shopify/sorbet/issues/362

        # Prism doesn't have enough context to reject f(&b, 1), and attempts to treat it equivalently to f(1, &b)
        "testdata/desugar/bad_block_pass.rb",

        # Additional failing Prism tests - various location and desugaring differences
        "testdata/compiler/block_iseq_names.rb",
        "testdata/compiler/exceptions/raise_across_interop__*.rb",
        "testdata/compiler/exceptions/rescue_in_block.rb",
        "testdata/compiler/intrinsics/array_compact.rb",
        "testdata/compiler/intrinsics/array_compact_bang.rb",
        "testdata/compiler/intrinsics/array_reject_bang.rb",
        "testdata/compiler/intrinsics/hash_merge.rb",
        "testdata/compiler/intrinsics/hash_merge_bang.rb",
        "testdata/core/fuzz_unparsable.rb",
        "testdata/desugar/fuzz_block_pass.rb",
        "testdata/desugar/fuzz_break_do_end.rb",
        "testdata/desugar/pattern_matching_const.rb",
        "testdata/desugar/pattern_matching_pins.rb",
        "testdata/infer/fuzz_uninitialized_vars.rb",
        "testdata/infer/missing_method_name.rb",
        "testdata/infer/or_and_conditional_branch.rb",
        "testdata/infer/t_proc_metatype.rb",
        "testdata/lsp/code_actions/extract_variable_single/invalid.rb",
        "testdata/lsp/completion/error_recovery_binop_unop.rb",
        "testdata/lsp/completion/if_empty.rb",
        "testdata/lsp/completion/instance_var.rb",
        "testdata/lsp/completion/instance_var_strict.rb",
        "testdata/lsp/completion/method_kwargs.rb",
        "testdata/lsp/completion/method_kwargs_extra.rb",
        "testdata/lsp/completion/missing_assign_rhs.rb",
        "testdata/lsp/completion/operators.rb",
        "testdata/lsp/completion/overridable.rb",
        "testdata/lsp/completion/setter.rb",
        "testdata/lsp/completion/snippet_before_variable.rb",
        "testdata/lsp/completion/superclass.rb",
        "testdata/lsp/completion/symbol_proc.rb",
        "testdata/lsp/completion/untyped_parse_errors.rb",
        "testdata/lsp/completion/untyped_receiver.rb",
        "testdata/lsp/hover_methods_inside_block.rb",
        "testdata/lsp/show_symbol.rb",
        "testdata/namer/duplicate_field_error.rb",
        "testdata/parser/bad_argument_crash.rb",
        "testdata/parser/compare_overload_parse_error.rb",
        "testdata/parser/crash_case_empty_var.rb",
        "testdata/parser/curly_braces_block_pass.rb",
        "testdata/parser/fuzz_ivar.rb",
        "testdata/parser/heredoc_squiggle_dedent.rb",
        "testdata/parser/heredoc_xstring.rb",
        "testdata/parser/it_param_invalid_syntax.rb",
        "testdata/parser/it_param_with_explicit_args.rb",
        "testdata/parser/kwargs_missing_comma_unsupported.rb",
        "testdata/resolver/it_param_rspec_method.rb",
        "testdata/resolver/sig_misc.rb",
        "testdata/rewriter/initializer_no_arg_with_local.rb",
        "testdata/desugar/op_asgn_kw_block.rb",

        # Prism desugared tree doesn't match legacy desugared tree (desugar differences)
        "testdata/desugar/destructure_rest.rb",
        "testdata/desugar/splat.rb",
        "testdata/desugar/star_in_block_arg.rb",
        "testdata/infer/generics/validator.rb",
        "testdata/lsp/fast_path/alias_stub_module__*.rb",
        "testdata/lsp/fast_path/not_enough_files/not_enough_files__*.rb",
        "testdata/lsp/fast_path/too_many_files/too_many_files__*.rb",
        "testdata/lsp/ripper_parse_error_formatting.rb",
        "testdata/lsp/rubyfmt_init_error.rb",
        "testdata/lsp/rubyfmt_io_error_formatting.rb",
        "testdata/lsp/rubyfmt_ruby_error.rb",
        "testdata/lsp/rubyfmt_unknown_error.rb",
        "testdata/lsp/successful_formatting.rb",
        "testdata/lsp/syntax_error_formatting.rb",
        "testdata/lsp/workspace_symbols_stdlib.rb",
        "testdata/minimize-rbi/external.rb",
        "testdata/minimize-rbi/private.rb",
        "testdata/packager/empty_package_file/**/*.rb",
        "testdata/packager/package-no-spec/**/*.rb",
        "testdata/packager/unpackaged-error/**/*.rb",
        "testdata/parser/empty.rb",
        "testdata/infer/crash_after_parse_errors.rb",
        "testdata/infer/untyped.rb",
        "testdata/parser/error_recovery/eof_8.rb",
        "testdata/parser/error_recovery/other_missing_end.rb",
        "testdata/rbi/proc.rb",

        # Prism preserves more of the structure than the legacy parser
        "testdata/parser/error_recovery/block_forwarding_invalid_def.rb",

        # Fixing a bug in pipeline_test_runner.cc revealed that these tests
        # were not actually running because they use MLHS
        "testdata/cfg/rescue_var_expression.rb",
        "testdata/parser/misc.rb",

        # Fixing a bug in pipeline_test_runner.cc revealed that these tests
        # were not actually running because they have RBS support enabled
        "testdata/lsp/hover_rbs.rb",
        "testdata/lsp/hover_rbs_assertions_let.rb",
    ],
    parser = "prism",
    test_name_prefix = "PrismPosTests",
    tests = glob(
        [
            # Replace [parser] with other phases to test Prism at that level
            # Phases: https://github.com/Shopify/sorbet/blob/prism/docs/internals.md#phases
            "testdata/**/*.rb",
            "testdata/**/*.exp",
        ],
        exclude = [
            # RBS tests have their own dedicated targets (rbs_prism)
            "testdata/rbs/**/*.rb",
            "testdata/rbs/**/*.exp",
        ],
    ),
)

pipeline_tests(
    "whitequark_parser_corpus",
    test_name_prefix = "WhitequarkParserTests",
    tests = glob([
        "whitequark/**/*.rb",
        "whitequark/**/*.exp",
    ]),
)

pipeline_tests(
    "test_corpus_lsp",
    extra_files = ["testdata/lsp/rubyfmt-stub/rubyfmt"],
    test_name_prefix = "LSPTests",
    tests = glob([
        "testdata/**/*.rb",
        "testdata/**/*.rbi",
        "testdata/**/*.exp",
    ]),
)

pipeline_tests(
    "test_corpus_lsp_prism",
    extra_files = ["testdata/lsp/rubyfmt-stub/rubyfmt"],
    parser = "prism",
    test_name_prefix = "PrismLSPTests",
    tests = glob(
        [
            "testdata/**/*.rb",
            "testdata/**/*.rbi",
            "testdata/**/*.exp",
        ],
        exclude = [
            # Prism LSP known failures: completion differences
            "testdata/lsp/completion/bad_arguments.rb",
            "testdata/lsp/completion/bad_list_elems.rb",
            "testdata/lsp/completion/case_1.rb",
            "testdata/lsp/completion/error_recovery_binop_unop.rb",
            "testdata/lsp/completion/instance_var.rb",
            "testdata/lsp/completion/method_kwargs.rb",
            "testdata/lsp/completion/method_kwargs_extra.rb",
            "testdata/lsp/completion/missing_assign_rhs.rb",
            "testdata/lsp/completion/missing_const_name.rb",
            "testdata/lsp/completion/overridable.rb",
            "testdata/lsp/completion/symbol_proc.rb",
            "testdata/lsp/completion/untyped_parse_errors.rb",

            # Prism LSP known failures: fast path
            "testdata/lsp/fast_path/static_field_type.rb",

            # Tests that are also disabled in the pipeline test
            "testdata/cfg/rescue_var_expression.rb",

            # Prism LSP known failures: hover with RBS
            "testdata/lsp/hover_rbs.rb",
            "testdata/lsp/hover_rbs_assertions_let.rb",
        ],
    ),
)

pipeline_tests(
    "prism_regression_corpus",
    parser = "prism",
    test_name_prefix = "PrismPosTests",
    tests = glob(
        [
            "prism_regression/**/*.rb",
            "prism_regression/**/*.exp",
        ],
        exclude = [
            # related to ruby 3.4 syntax changes
            "prism_regression/assign_to_index.rb",
            "prism_regression/call_block_param.rb",
        ],
    ),
)

pipeline_tests(
    "rbs_whitequark",
    disabled_tests = [
        # Specific test that is modified for Prism, contains minor differences in the exp file
        "testdata/rbs/assertions_heredoc_modified.rb",
    ],
    test_name_prefix = "PosTests",
    tests = glob([
        "testdata/rbs/**/*.rb",
        "testdata/rbs/**/*.exp",
    ]),
)

pipeline_tests(
    "rbs_prism",
    disabled_tests = [
        # Specific test for the legacy pipeline. Prism has its own `testdata/rbs/assertions_heredoc_modified.rb`.
        "testdata/rbs/assertions_heredoc.rb",

        # Prism output differs from legacy for constant resolution.
        "testdata/rbs/signatures_types.rb",
    ],
    parser = "prism",
    test_name_prefix = "PrismPosTests",
    tests = glob([
        "testdata/rbs/**/*.rb",
        "testdata/rbs/**/*.exp",
    ]),
)

test_suite(
    name = "test",
    tests = [
        "rbs_whitequark",
        "test_corpus",
    ],
)

test_suite(
    name = "test_prism",
    tests = [
        "rbs_prism",
        "test_corpus_prism",
    ],
)

test_suite(
    name = "whitequark_parser_tests",
    tests = ["whitequark_parser_corpus"],
)

test_suite(
    name = "prism_regression",
    tests = ["prism_regression_corpus"],
)

test_suite(
    name = "rbs",
    tests = [
        "rbs_prism",
        "rbs_whitequark",
    ],
)
