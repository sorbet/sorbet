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
                }
                tests[test_name] = data

    enabled_tests = []
    disabled_tests = []
    for name in tests.keys():
        test_name = "test_{}/{}".format(test_name_prefix, name)
        path = tests[name]["path"]
        prefix = tests[name]["prefix"]
        sentinel = tests[name]["sentinel"]
        sources_name = "test_{}/{}_rb_source".format(test_name_prefix, name)
        exps_name = "test_{}/{}_exp".format(test_name_prefix, name)
        expected_outfile = "{}.out".format(prefix)
        expected_exitfile = "{}.exit".format(prefix)
        build_archive = "{}.tar.gz".format(prefix)

        # determine if we need to mark this as a manual test
        extra_tags = []
        if tests[name]["disabled"]:
            extra_tags = ["manual"]
            disabled_tests.append(test_name)
        else:
            enabled_tests.append(test_name)

        # All of the expectations (if this test is a single file)
        if tests[name]["isMultiFile"]:
            exp_sources = []
            test_sources = native.glob(["{}*.rb".format(prefix)])
        else:
            exp_sources = native.glob(["{}.*.exp".format(path)])
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

        native.genrule(
            name = "test_{}/{}_gen_output".format(test_name_prefix, name),
            outs = [expected_outfile, expected_exitfile],
            srcs = [sources_name],
            tools = [":generate_out_file"],
            cmd = "$(location :generate_out_file) $(location {}) $(location {}) $(locations {})".format(expected_outfile, expected_exitfile, sources_name),
            tags = tags + extra_tags,
        )

        native.genrule(
            name = "test_{}/{}_build".format(test_name_prefix, name),
            outs = [build_archive],
            srcs = [sources_name],
            tools = [":build_extension"],
            cmd =
                """
                $(location :build_extension) $(location {}) $(locations {}) &> genrule.log \
                    || (cat genrule.log && exit 1)
                """.format(build_archive, sources_name),
            tags = tags + extra_tags,
        )

        data = [
            "//run:runtimeoverrides",
            "@ruby_2_6_3//:ruby",
            build_archive,
            expected_outfile,
            expected_exitfile,
            sources_name,
            exps_name,
        ]

        native.sh_test(
            name = "test_{}/{}".format(test_name_prefix, name),
            srcs = ["test_corpus_runner.sh"],
            deps = [":logging"],
            args = [
                "--expected_output=$(location {})".format(expected_outfile),
                "--expected_exit_code=$(location {})".format(expected_exitfile),
                "--build_archive=$(location {})".format(build_archive),
                "--ruby=$(location @ruby_2_6_3//:ruby)",
                "$(locations {})".format(sources_name),
            ] + extra_args,
            data = data,
            size = "large",
            tags = tags + extra_tags,
        )

    native.test_suite(
        name = suite_name,
        tests = enabled_tests,
    )

    native.test_suite(
        name = "{}_disabled".format(suite_name),
        tests = disabled_tests,
        tags = ["manual"],
    )
