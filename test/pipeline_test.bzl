def basename(p):
    return p.rpartition("/")[-1]

def dirname(p):
    dirname, sep, fname = p.rpartition("/")
    if dirname:
        return dirname.rstrip("/")
    else:
        return sep

def dropExtension(p):
    "TODO: handle multiple . in name"
    return p.partition(".")[0]

_TEST_SCRIPT = """#!/usr/bin/env bash
exec {runner} --single_test="{test}"
"""

def _exp_test_impl(ctx):
    ctx.actions.write(
        output = ctx.outputs.executable,
        content = _TEST_SCRIPT.format(
            runner = ctx.executable.runner.short_path,
            test = ctx.file.test.path,
        )
    )

    runfiles = ctx.runfiles(files = ctx.files.runner + ctx.files.test + ctx.files.data)

    return [DefaultInfo(runfiles = runfiles)]

exp_test = rule(
    implementation = _exp_test_impl,
    test = True,
    attrs = {

        "test": attr.label(
            allow_single_file = True,
        ),

        "data": attr.label_list(
            allow_files = True,
        ),

        "runner": attr.label(
            default = ":cli_test_runner",
            executable = True,
            cfg = "host",
            allow_files = True,
        ),
    },
)

_TEST_RUNNERS = {
    "PosTests": ":cli_test_runner",
    "LspTests": ":lsp_test_runner",
    "WhitequarkParserTests": ":parser_test_runner",
}

def pipeline_tests(suite_name, all_paths, test_name_prefix, filter = "*", extra_args = [], tags = []):
    tests = {}  # test_name-> {"path": String, "prefix": String, "sentinel": String}

    for path in all_paths:
        if path.endswith(".rb") or path.endswith(".rbi"):
            prefix = dropExtension(basename(path).partition("__")[0])
            test_name = dirname(path) + "/" + prefix

            #test_name = test_name.replace("/", "_")
            current = tests.get(test_name)
            if None == current:
                data = {
                    "path": dirname(path),
                    "prefix": "{}/{}".format(dirname(path), prefix),
                    "sentinel": path,
                    "isMultiFile": "__" in path,
                    "disabled": "disabled" in path,
                }
                tests[test_name] = data

    enabled_tests = []
    disabled_tests = []
    for name in tests.keys():
        test_name = "test_{}/{}".format(test_name_prefix, name)
        path = tests[name]["path"]
        prefix = tests[name]["prefix"]
        sentinel = tests[name]["sentinel"]

        # determine if we need to mark this as a manual test
        extra_tags = []
        if tests[name]["disabled"]:
            extra_tags = ["manual"]
            disabled_tests.append(test_name)
        else:
            enabled_tests.append(test_name)

        data = ["cli_test_runner"]
        if tests[name]["isMultiFile"]:
            data += native.glob(["{}*".format(prefix)])
        else:
            data += [sentinel]
            data += native.glob(["{}.*.exp".format(prefix)])
            data += native.glob(["{}.*.rbupdate".format(prefix)])
            data += native.glob(["{}.*.rbedited".format(prefix)])

        exp_test(
            name = "test_{}/{}".format(test_name_prefix, name),
            data = data,
            test = sentinel,
            size = "small",
            tags = tags + extra_tags,
        )

    native.test_suite(
        name = suite_name,
        tests = enabled_tests,
    )

    native.test_suite(
        name = "{}_disabled".format(suite_name),
        tests = disabled_tests,
        tags = ["manual"],
    )
