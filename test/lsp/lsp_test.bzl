def lsp_test(path):
    # path will be like `$name/$name.sh`
    words = path.split("/")
    name = words[-2]
    if words[-1] != "{}.rec".format(name):
        fail("cli test scripts must be named cli/$name/$name.rec")

    native.sh_test(
        name = "test_{}".format(name),
        srcs = ["lsp_test_runner.sh"],
        args = ["$(location {})".format(path)],
        data = [
            path,
            "//main:sorbet"
        ],
        size = 'small',
    )


    native.sh_test(
            name = "update_{}".format(name),
            srcs = ["update_one.sh"],
            args = ["$(location {})".format(path), "$(location {})".format("lsp_test_runner.sh")],
            data = [
                "lsp_test_runner.sh",
                path,
                "//main:sorbet"
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
