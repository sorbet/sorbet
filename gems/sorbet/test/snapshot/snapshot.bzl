def snapshot_tests(ruby = None, tests = []):
    """
    Define a bunch of snapshot tests all at once. The paths must all be of the
    form "partial/<test>" or "total/<test>".
    """

    if ruby == None:
        fail(msg = "No ruby version specified in snapshot_tests")

    name = "snapshot-{}".format(ruby)
    update_name = "update-{}".format(ruby)

    test_rules = []
    update_rules = []

    for test_path in tests:
        res = _snapshot_test(test_path, ruby)
        test_rules.append(res["test_name"])

        if res.get("update_name") != None:
            update_rules.append(res["update_name"])

    native.test_suite(
        name = name,
        tests = test_rules,
        # NOTE: ensure that this rule isn't caught in //...
        tags = ["manual"],
    )

    native.test_suite(
        name = update_name,
        tests = update_rules,
        # NOTE: ensure that this rule isn't caught in //...
        tags = ["manual"],
    )

def _snapshot_test(test_path, ruby):
    """
    test_path is of the form `total/test` or `partial/test`.
    ruby is the version of ruby to use (named in //third_party/externals.bzl)
    """

    res = {}
    actual = "{}/actual_{}.tar.gz".format(test_path, ruby)

    native.genrule(
        name = "actual_{}/{}".format(ruby, test_path),
        message = "Running {} ({})".format(test_path, ruby),
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

        # NOTE: this is manual to avoid being caught with `//...`
        tags = ["manual"],

        # NOTE: this redirects stdout/stderr to a log, and only outputs on
        # failure.
        cmd =
            """
        $(location :run_one_bazel) {} $(location {}) {} &> genrule.log \
                || (cat genrule.log && exit 1)
        """.format(ruby, actual, test_path),
    )

    test_name = "test_{}/{}".format(ruby, test_path)

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
            test_path,
            "$(location {})".format(actual),
        ],

        # NOTE: this is manual to avoid being caught with `//...`
        tags = ["manual"],
    )

    res["test_name"] = test_name

    # Generate an update rule if the test has an expected directory
    expected = native.glob(
        ["{}/expected".format(test_path)],
        exclude_directories = 0,
    )

    if len(expected) > 0:
        update_name = "update_{}/{}".format(ruby, test_path)

        native.sh_test(
            name = update_name,
            data = [actual] + expected,
            deps = [":logging"],
            srcs = ["update_one_bazel.sh"],
            args = [
                "$(location {})".format(actual),
                "$(location {}/expected)".format(test_path),
                test_path,
            ],

            # Don't run this rule remotely, or in the sandbox
            local = True,

            # NOTE: these tags cause this test to be skipped by `bazel test //...`,
            # and not run in the sandbox:
            #
            # "manual"   - don't include this rule in `bazel test //...`
            # "external" - unconditionally execute this rule
            tags = [
                "manual",
                "external",
            ],
        )

        res["update_name"] = update_name

    return res
