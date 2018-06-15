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
