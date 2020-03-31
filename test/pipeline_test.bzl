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

def pipeline_tests(suite_name, all_paths, test_name_prefix, filter = "*", extra_args = [], tags = []):
    tests = {}  # test_name-> {"path": String, "prefix": String, "sentinel": String}

    for path in all_paths:
        if path.endswith(".rb") or path.endswith(".rbi"):
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
        else:
            data += [sentinel]
            data += native.glob(["{}.*.exp".format(prefix)])
            data += native.glob(["{}.*.rbupdate".format(prefix)])
            data += native.glob(["{}.*.rbedited".format(prefix)])

        native.sh_test(
            name = "test_{}/{}".format(test_name_prefix, name),
            srcs = ["test_corpus_forwarder.sh"],
            deps = ["@bazel_tools//tools/bash/runfiles"],
            args = ["--single_test=$(location {})".format(sentinel), "--gtest_filter={}/*".format(filter)] + extra_args,
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
