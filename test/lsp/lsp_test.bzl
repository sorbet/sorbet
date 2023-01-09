def basename(p):
    return p.rpartition("/")[-1]

def dropExtension(p):
    "TODO: handle multiple . in name"
    return p.partition(".")[0]

def lsp_test(path):
    # path will be like `$name/$name.sh`
    words = path.split("/")
    name = words[-2]
    test_name = "test_{}".format(name)
    if words[-1] != "{}.rec".format(name):
        fail("cli test scripts must be named cli/$name/$name.rec")

    native.sh_test(
        name = test_name,
        srcs = ["lsp_test_runner.sh"],
        args = ["$(location {})".format(path)],
        data = [
            path,
            "//main:sorbet",
        ],
        size = "small",
    )

    native.sh_test(
        name = "update_{}".format(name),
        srcs = ["update_one.sh"],
        args = ["$(location {})".format(path), "$(location {})".format("lsp_test_runner.sh")],
        data = [
            "lsp_test_runner.sh",
            path,
            "//main:sorbet",
        ],
        tags = [
            "manual",
            "external",
            "local",
        ],
        size = "small",
    )

    return test_name

def update_test():
    existing = native.existing_rules()
    update_rules = [
        rule
        for (rule, data) in existing.items()
        if rule.startswith("update_") and data["kind"] == "sh_test"
    ]
    native.test_suite(
        name = "update",
        tags = ["manual"],
        tests = update_rules,
    )

_PROTOCOL_TEST_SRCS = ["ProtocolTest.cc", "ProtocolTest.h"]

def protocol_tests(cc_files):
    for cc_file in cc_files:
        native.cc_test(
            name = dropExtension(basename(cc_file)),
            timeout = "moderate",
            srcs = _PROTOCOL_TEST_SRCS + [cc_file],
            linkstatic = select({
                "//tools/config:linkshared": 0,
                "//conditions:default": 1,
            }),
            visibility = ["//tools:__pkg__"],
            deps = [
                "//core",
                "//main/lsp",
                "//payload",
                "//test/helpers",
                "@doctest//doctest",
                "@doctest//doctest:main",
            ],
        )
