def basename(p):
    return p.rpartition("/")[-1]

def dirname(p):
    dirname, sep, fname = p.rpartition("/")
    if dirname:
        return dirname.rstrip("/")
    else:
        return sep

def dropExtension(p):
    "TODO: handle multiple . in name"
    return p.partition(".")[0]

def compiler_tests(suite_name, all_paths, test_name_prefix = "PosTests", extra_args = [], tags = []):
    tests = {}  # test_name-> {"path": String, "prefix": String, "sentinel": String}

    for path in all_paths:
        if path.endswith(".rb"):
            prefix = dropExtension(basename(path).partition("__")[0])
            test_name = dirname(path) + "/" + prefix

            #test_name = test_name.replace("/", "_")
            current = tests.get(test_name)
            if None == current:
                data = {
                    "path": dirname(path),
                    "prefix": "{}/{}".format(dirname(path), prefix),
                    "sentinel": path,
                    "isMultiFile": "__" in path,
                    "disabled": "disabled" in path,
                    "too_slow": "too_slow" in path,
                }
                tests[test_name] = data

    enabled_tests = []
    too_slow_tests = []
    for name in tests.keys():
        test_name = "test_{}/{}".format(test_name_prefix, name)
        validate_exp = "validate_exp_{}/{}".format(test_name_prefix, name)

        path = tests[name]["path"]
        prefix = tests[name]["prefix"]
        sentinel = tests[name]["sentinel"]
        sources_name = "test_{}/{}_rb_source".format(test_name_prefix, name)
        exps_name = "test_{}/{}_exp".format(test_name_prefix, name)
        expected_outfile = "{}.out".format(prefix)
        expected_errfile = "{}.err".format(prefix)
        expected_exitfile = "{}.exit".format(prefix)
        build_archive = "{}.tar".format(prefix)

        # All of the expectations (if this test is a single file)
        if tests[name]["isMultiFile"]:
            exp_sources = []
            test_sources = native.glob(["{}*.rb".format(prefix)])
        else:
            exp_sources = native.glob(["{}.*.exp".format(prefix)])
            test_sources = [sentinel]

        # All of the test sources
        native.filegroup(
            name = sources_name,
            srcs = test_sources,
            visibility = ["//visibility:public"],
        )

        native.filegroup(
            name = exps_name,
            srcs = exp_sources,
            visibility = ["//visibility:public"],
        )

        # Mark the too_slow tests as manual
        extra_tags = []
        if tests[name]["too_slow"]:
            extra_tags = ["manual"]

        ruby_genrule = "test_{}/{}_gen_output".format(test_name_prefix, name)
        native.genrule(
            name = ruby_genrule,
            outs = [expected_outfile, expected_errfile, expected_exitfile],
            srcs = [sources_name],
            tools = [":generate_out_file"],
            cmd =
                """
                $(location :generate_out_file) \
                        $(location {}) \
                        $(location {}) \
                        $(location {}) \
                        $(locations {}) \
                        2>&1 > genrule.log || \
                        cat genrule.log
                """.format(expected_outfile, expected_errfile, expected_exitfile, sources_name),
            tags = tags + extra_tags,
        )

        native.genrule(
            name = "test_{}/{}_build".format(test_name_prefix, name),
            outs = [build_archive],
            srcs = [sources_name],
            tools = [":build_extension"],
            cmd =
                """
                $(location :build_extension) $(location {}) $(locations {}) \
                  2>&1 > genrule.log || \
                  cat genrule.log
                """.format(build_archive, sources_name),
            tags = tags + extra_tags,
        )

        defined_tests = [test_name]

        native.sh_test(
            name = test_name,
            srcs = ["test_corpus_runner.sh"],
            deps = [":logging"],
            args = [
                "--expected_output=$(location {})".format(expected_outfile),
                "--expected_err=$(location {})".format(expected_errfile),
                "--expected_exit_code=$(location {})".format(expected_exitfile),
                "--build_archive=$(location {})".format(build_archive),
                "--ruby=$(location @sorbet_ruby//:ruby)",
                "--expect-fail={}".format(tests[name]["disabled"]),
                "$(locations {})".format(sources_name),
            ] + extra_args,
            data = [
                "patch_require.rb",
                "@sorbet_ruby//:ruby",
                "@com_stripe_ruby_typer//gems/sorbet-runtime",
                build_archive,
                expected_outfile,
                expected_errfile,
                expected_exitfile,
                sources_name,
            ],
            size = "small",
            tags = tags + extra_tags,
        )

        if len(exp_sources) > 0:
            native.sh_test(
                name = validate_exp,
                srcs = ["validate_exp.sh"],
                deps = [":logging"],
                args = [
                    "--build_archive=$(location {})".format(build_archive),
                    "$(locations {})".format(sources_name),
                ] + extra_args,
                data = [
                    build_archive,
                    sources_name,
                    exps_name,
                    ":llvm-diff",
                ],
                size = "small",
                tags = tags + extra_tags,
            )

            defined_tests.append(validate_exp)

        if tests[name]["too_slow"]:
            too_slow_tests.extend(defined_tests)
        else:
            enabled_tests.extend(defined_tests)

    native.test_suite(
        name = suite_name,
        tests = enabled_tests,
    )

    native.test_suite(
        name = "{}_disabled".format(suite_name),
        tests = too_slow_tests,
        tags = ["manual"],
    )
