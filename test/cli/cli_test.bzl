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
