def cli_test(path):
    # path will be like `$name/$name.sh`
    words = path.split("/")
    name = words[-2]
    if words[-1] != "{}.sh".format(name):
        fail("cli test scripts must be named cli/$name/$name.sh")

    native.sh_binary(
        name = "run_{}".format(name),
        srcs = [path],
        data = native.glob([
            "{}/*.rb".format(name),
            "{}/*.yaml".format(name),
            "{}/*.input".format(name),
        ]) + ["//main:sorbet"]
    )

    output = path.replace('.sh', '.out')

    native.sh_test(
        name = "test_{}".format(name),
        srcs = ["test_one.sh"],
        args = ["$(location {})".format(path), "$(location {})".format(output)],
        data = [
            path,
            ":run_{}".format(name),
            output,
        ],
        size = 'small',
    )

    native.sh_test(
        name = "update_{}".format(name),
        srcs = ["update_one.sh"],
        args = ["$(location {})".format(path), "$(location {})".format(output)],
        data = [
            path,
            ":run_{}".format(name),
            output,
        ],
        tags = [
            "manual", "external", "local",
        ],
        size = 'small',
    )

def update_test():
    existing = native.existing_rules()
    update_rules = [rule for (rule, data) in existing.items()
                    if rule.startswith("update_") and data['kind'] == 'sh_test']
    native.test_suite(
        name = "update",
        tags = [
            "manual", "external", "local",
        ],
        tests = update_rules,
    )
