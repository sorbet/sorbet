def _verify_single_test_script(scripts):
    # build a mapping from name => test script
    mapping = {}

    # script will be "$name/$name.sh"
    for script in scripts:
        words = script.split("/")
        name = words[-2]
        script_file = words[-1]
        expected_file = "test.sh"
        if script_file != expected_file:
            fail("cli test scripts must be named cli/$name/test.sh")

        existing = mapping.get(name)
        if existing != None:
            fail("cli tests must have a single shell script in their top-level directory")
        mapping[name] = script_file

    return mapping

def cli_tests(suite_name, scripts, tags = []):
    tests = []
    updates = []

    mapping = _verify_single_test_script(scripts)

    for name, script in mapping.items():
        test_name, update_name = _cli_test(name, script, tags)
        tests.append(test_name)
        updates.append(update_name)

    native.test_suite(
        name = suite_name,
        tests = tests,
    )

    native.test_suite(
        name = "update",
        tags = ["manual"],
        tests = updates,
    )

def _cli_test(name, script, tags = []):
    test_name = "test_{}".format(name)

    script_path = "{}/{}".format(name, script)

    native.sh_binary(
        name = "run_{}".format(name),
        srcs = [script_path],
        data = native.glob([
            "{}/**/*.rb".format(name),
            "{}/**/*.rbi".format(name),
            "{}/**/*.rake".format(name),
            "{}/**/*.ru".format(name),
            "{}/*.rbi".format(name),
            "{}/*.yaml".format(name),
            "{}/*.input".format(name),
            "{}/sorbet/*".format(name),
            "{}/**/file_with_no_dot".format(name),
        ]) + ["//main:sorbet", "@com_google_protobuf//:protoc", "//proto:protos"],
    )

    output = script_path.replace(".sh", ".out")

    native.sh_test(
        name = test_name,
        srcs = ["test_one.sh"],
        args = ["$(location {})".format(script_path), "$(location {})".format(output)],
        data = [
            script_path,
            ":run_{}".format(name),
            output,
            "//test/cli:llvm-symbolizer",
        ],
        size = "medium",
        tags = tags,
    )

    update_name = "update_{}".format(name)

    native.sh_test(
        name = update_name,
        srcs = ["update_one.sh"],
        args = ["$(location {})".format(script_path), "$(location {})".format(output)],
        data = [
            script_path,
            ":run_{}".format(name),
            output,
        ],
        tags = [
            "manual",
            "external",
            "local",
        ] + tags,
        size = "small",
    )

    return test_name, update_name
