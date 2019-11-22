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
        filegroup_name = "test_{}/{}_rb_file".format(test_name_prefix, name)
        expected_outfile = "{}.out".format(prefix)

        # determine if we need to mark this as a manual test
        extra_tags = []
        if tests[name]["disabled"]:
            extra_tags = ["manual"]
            disabled_tests.append(test_name)
        else:
            enabled_tests.append(test_name)

        data = ["test_corpus_runner"]
        if tests[name]["isMultiFile"]:
            data += native.glob(["{}*".format(prefix)])
            data += [expected_outfile]
        else:
            data += [sentinel, expected_outfile]
            data += native.glob(["{}.*.exp".format(prefix)])

        native.filegroup(
            name = filegroup_name,
            srcs = [sentinel],
            visibility = ["//visibility:public"],
        )

        native.genrule(
            name = "test_{}/{}_gen_output".format(test_name_prefix, name),
            outs = [expected_outfile],
            srcs = [filegroup_name],
            tools = [":generate_out_file"],
            cmd = "$(location :generate_out_file) $(location {}) $(locations {})".format(expected_outfile, filegroup_name),
            tags = tags + extra_tags,
        )

        native.sh_test(
            name = "test_{}/{}".format(test_name_prefix, name),
            srcs = ["test_corpus_forwarder.sh"],
            args = ["--single_test=$(location {})".format(sentinel), "--expected_output=$(location {})".format(expected_outfile)] + extra_args,
            data = data,
            size = "small",
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
