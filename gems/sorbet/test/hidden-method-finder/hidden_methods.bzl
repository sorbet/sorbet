def hidden_methods_tests(ruby = None, tests = []):
    """
    Define a bunch of hidden method tests all at once.
    """

    if ruby == None:
        fail(msg = "No ruby version specified in hidden_methods_tests")

    name = "hidden-methods-{}".format(ruby)
    update_name = "hidden-methods-update-{}".format(ruby)

    test_rules = []
    update_rules = []

    for test_path in tests:
        res = _hidden_methods_test(test_path, ruby)
        test_rules.append(res["test_name"])
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

def _hidden_methods_test(test_path, ruby):
    """
    ruby is the version of ruby to use (named in //third_party/externals.bzl)
    """

    res = {}
    actual = "{}/actual_{}.rbi".format(test_path, ruby)
    expected = "{}/{}_hidden.rbi.exp".format(test_path, ruby)

    native.genrule(
        name = "actual_{}/{}".format(ruby, test_path),
        message = "Running {} ({})".format(test_path, ruby),
        tools = [
            ":hidden_methods_bazel",
        ],
        srcs = [
            "//main:sorbet",
            "//gems/sorbet:sorbet",
            expected,

            # TODO: how do we ask for the environment of the c compiler here,
            # for supporting gems with native code? (An example of this is the
            # `json` gem)
        ] + native.glob(
            [
                "{}/src/**/*".format(test_path),
            ],
        ),
        outs = [actual],

        # NOTE: this is manual to avoid being caught with `//...`
        tags = ["manual"],

        # NOTE: this redirects stdout/stderr to a log, and only outputs on
        # failure.
        cmd =
            """
        $(location :hidden_methods_bazel) {} $(location {}) {} &> genrule.log \
                || (cat genrule.log && exit 1)
        """.format(ruby, actual, test_path),
    )

    test_name = "test_{}/{}".format(ruby, test_path)

    native.sh_test(
        name = test_name,
        srcs = ["check_one_bazel.sh"],
        data = [actual, expected] + native.glob([
            "{}/src/**/*".format(test_path),
        ]),
        deps = [":logging"],
        args = [
            "$(location {})".format(actual),
            test_path,
            ruby,
        ],

        # NOTE: this is manual to avoid being caught with `//...`
        tags = ["manual"],
    )

    res["test_name"] = test_name

    update_name = "update_{}/{}".format(ruby, test_path)

    native.sh_test(
        name = update_name,
        data = [actual, expected],
        deps = [":logging"],
        srcs = ["update_one_bazel.sh"],
        args = [
            "$(location {})".format(actual),
            test_path,
            ruby,
            "$(location {})".format(expected),
        ],

        # NOTE: these tags cause this test to be skipped by `bazel test //...`,
        # and not run in the sandbox:
        #
        # "manual"   - don't include this rule in `bazel test //...`
        # "external" - unconditionally execute this rule
        tags = [
            "manual",
            "external",
            "local",
        ],
    )

    res["update_name"] = update_name

    return res
