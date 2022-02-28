def cli_tests(suite_name, scripts, tags = []):
    tests = []
    updates = []
    for script in scripts:
        test_name, update_name = cli_test(script, tags)
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

def cli_test(path, tags = []):
    # path will be like `$name/$name.sh`
    words = path.split("/")
    name = words[-2]
    test_name = "test_{}".format(name)
    if words[-1] != "{}.sh".format(name):
        fail("cli test scripts must be named cli/$name/$name.sh")

    native.sh_binary(
        name = "run_{}".format(name),
        srcs = [path],
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

    output = path.replace(".sh", ".out")

    native.sh_test(
        name = test_name,
        srcs = ["test_one.sh"],
        args = ["$(location {})".format(path), "$(location {})".format(output)],
        data = [
            path,
            ":run_{}".format(name),
            output,
        ],
        size = "medium",
        tags = tags,
    )

    update_name = "update_{}".format(name)

    native.sh_test(
        name = update_name,
        srcs = ["update_one.sh"],
        args = ["$(location {})".format(path), "$(location {})".format(output)],
        data = [
            path,
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
