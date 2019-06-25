def snapshot_tests(name = "snapshot", update_name = "update", tests = []):
    """
    Define a bunch of snapshot tests all at once. The paths must all be of the
    form "partial/<test>" or "total/<test>".
    """

    test_rules = []
    update_rules = []

    for test_path in tests:
        res = _snapshot_test(test_path)
        test_rules.append(res["test_name"])

        if res.get("update_name") != None:
            update_rules.append(res["update_name"])

    native.test_suite(
        name = name,
        tests = test_rules,
    )

    native.test_suite(
        name = update_name,
        tests = update_rules,
    )

def _snapshot_test(test_path):
    """
    test_path is of the form `total/test` or `partial/test`.
    """

    res = {}
    actual = "{}/actual.tar.gz".format(test_path)

    native.genrule(
        name = "actual_{}".format(test_path),
        message = "Running {}".format(test_path),
        tools = [
            ":run_one_bazel",
        ],
        srcs = [
            "//main:sorbet",
            "//gems/sorbet:sorbet",

            # TODO: how do we ask for the environment of the c compiler here,
            # for supporting gems with native code? (An example of this is the
            # `json` gem)
        ] + native.glob(
            [
                "{}/src/**/*".format(test_path),
                "{}/expected/**/*".format(test_path),
                "{}/gems/**/*".format(test_path),
            ],
        ),
        outs = [actual],

        # NOTE: this redirects stdout/stderr to a log, and only outputs on
        # failure.
        cmd =
            """
        $(location :run_one_bazel) ruby_2_4_3 $(location {}) {} &> genrule.log \
                || (cat genrule.log && exit 1)
        """.format(actual, test_path),
    )

    test_name = "test_{}".format(test_path)

    native.sh_test(
        name = test_name,
        srcs = ["check_one_bazel.sh"],
        data = [actual] + native.glob([
            "{}/src/**/*".format(test_path),
            "{}/expected/**/*".format(test_path),
            "{}/gems/**/*".format(test_path),
        ]),
        deps = [":logging"],
        args = [
            "$(location {})".format(actual),
            test_path,
        ],
    )

    res["test_name"] = test_name

    # Generate an update rule if the test has an expected directory
    expected = native.glob(
        ["{}/expected".format(test_path)],
        exclude_directories = 0,
    )

    if len(expected) > 0:
        update_name = "update_{}".format(test_path)

        native.sh_test(
            name = "update_{}".format(test_path),
            data = [actual] + expected,
            deps = [":logging"],
            srcs = ["update_one_bazel.sh"],
            args = [
                "$(location {})".format(actual),
                "$(location {}/expected)".format(test_path),
                test_path,
            ],

            # NOTE: these tags cause this test to be skipped by `bazel test //...`,
            # and not run in the sandbox.
            tags = [
                "manual",
                "external",
                "local",
            ],
        )

        res["update_name"] = update_name

    return res
