def cli_tests(name, tests = [], tags = []):
    test_names = []
    actual_names = []
    for path in tests:
        test_dir = path.rsplit("/")[0]
        test_name = "test_{}".format(test_dir)
        run_test_name = "run_{}".format(test_dir)
        test_srcs = "srcs_{}".format(test_dir)
        expected_out = "expected_{}".format(test_dir)
        actual_out = "{}/actual.out".format(test_dir)

        native.filegroup(
            name = test_srcs,
            srcs = native.glob(["{}/*".format(test_dir)], exclude = ["expected.out"]),
        )

        native.filegroup(
            name = expected_out,
            srcs = ["{}/expected.out".format(test_dir)],
        )

        _cli_test_output(
            name = run_test_name,
            out = actual_out,
            path = test_dir,
            srcs = test_srcs,
            tags = tags,
        )

        _cli_test(
            name = test_name,
            output = run_test_name,
            expected = expected_out,
            tags = tags,
        )

        test_names.append(test_name)
        actual_names.append(actual_out)

    native.filegroup(
        name = "actual_{}".format(name),
        srcs = actual_names,
        tags = tags,
    )

    native.test_suite(
        name = name,
        tests = test_names,
    )

CliTestOutput = provider(fields = ["output_file"])

def _cli_test_output_impl(ctx):
    test_dir = "{}/{}".format(ctx.label.package, ctx.attr.path)

    output = ctx.outputs.out

    ctx.actions.run(
        outputs = [output],
        inputs = ctx.files.srcs,
        executable = ctx.executable._run_test,
        arguments = [
            test_dir,
            output.path,
        ],
    )

    return [CliTestOutput(output_file = output)]

_cli_test_output = rule(
    implementation = _cli_test_output_impl,
    attrs = {
        "srcs": attr.label(),
        "out": attr.output(),
        "path": attr.string(),
        "_run_test": attr.label(default = ":run_test", cfg = "target", executable = True),
    },
    provides = [CliTestOutput],
)

def _cli_test_impl(ctx):
    test_name = ctx.label.name

    script = ctx.actions.declare_file(test_name)

    output = ctx.attr.output[CliTestOutput].output_file

    ctx.actions.expand_template(
        template = ctx.file._validate_template,
        output = script,
        is_executable = True,
        substitutions = {
            "{expected_output}": ctx.file.expected.short_path,
            "{test_output}": output.short_path,
        },
    )

    runfiles = ctx.runfiles(files = [ctx.file.expected, output])

    return [DefaultInfo(executable = script, runfiles = runfiles)]

_cli_test = rule(
    implementation = _cli_test_impl,
    attrs = {
        "output": attr.label(providers = [CliTestOutput]),
        "expected": attr.label(allow_single_file = True),
        "_validate_template": attr.label(default = ":validate.sh.tpl", allow_single_file = True),
    },
    test = True,
)
