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

def compiler_tests(suite_name, all_paths, extra_args = [], tags = []):
    # test_name-> {"path": String, "sentinel": String, "isMultiFile": bool, "disabled": bool, "too_slow": bool}
    tests = {}

    for path in all_paths:
        if path.endswith(".rb"):
            prefix = dropExtension(basename(path).partition("__")[0])
            test_name = dirname(path) + "/" + prefix

            #test_name = test_name.replace("/", "_")
            current = tests.get(test_name)
            if None == current:
                data = {
                    "path": dirname(path),
                    "sentinel": path,
                    "isMultiFile": "__" in path,
                    "disabled": "disabled" in path,
                    "too_slow": "too_slow" in path,
                }
                tests[test_name] = data

    oracle_tests = []
    validate_tests = []
    filecheck_tests = []
    too_slow_tests = []
    for name in tests.keys():
        test_name = "test_{}".format(name)
        validate_exp = "validate_exp_{}".format(name)
        filecheck = "filecheck_{}".format(name)

        path = tests[name]["path"]
        sentinel = tests[name]["sentinel"]
        sources_name = "test_{}_rb_source".format(name)
        exps_name = "test_{}_exp".format(name)
        expected_outfile = "{}.ruby.stdout".format(name)
        expected_errfile = "{}.ruby.stderr".format(name)
        expected_exitfile = "{}.ruby.exit".format(name)
        build_dir = "{}.sorbet.build".format(name)
        sorbet_exitfile = "{}.sorbet.exit".format(name)
        sorbet_out = "{}.sorbet.stdout".format(name)

        # All of the expectations (if this test is a single file)
        if tests[name]["isMultiFile"]:
            exp_sources = []
            test_sources = native.glob(["{}__*.rb".format(name), "{}__*.rbi".format(name)])
        else:
            exp_sources = native.glob(["{}.*.exp".format(name)])
            test_sources = [sentinel]

        # All of the test sources
        native.filegroup(
            name = sources_name,
            srcs = test_sources,
            visibility = ["//visibility:public"],
        )

        native.filegroup(
            name = exps_name,
            srcs = exp_sources,
            visibility = ["//visibility:public"],
        )

        # Mark the too_slow tests as manual
        extra_tags = []
        if tests[name]["too_slow"]:
            extra_tags = ["manual"]

        oracle = "oracle_{}".format(name)
        build_ruby_oracle(
            name = oracle,
            prefix = name,
            test = sentinel,
            srcs = sources_name,
            log = "{}.ruby.log".format(name),
            stdout = expected_outfile,
            stderr = expected_errfile,
            exit = expected_exitfile,
            tags = tags + extra_tags,
        )

        extension = "extension_{}".format(name)
        build_extension(
            name = extension,
            prefix = name,
            srcs = sources_name,
            out = build_dir,
            log = "{}.sorbet.log".format(name),
            stdout = sorbet_out,
            exit = sorbet_exitfile,
            tags = tags + extra_tags,
        )

        validate_sorbet_output_test(
            name = test_name,
            srcs = sources_name,
            expected_failure = tests[name]["disabled"],
            build = extension,
            oracle = oracle,
            tags = tags + extra_tags,
            size = "small",
        )

        exp_tests = []
        if len(exp_sources) > 0:
            validate_exp_test(
                name = validate_exp,
                srcs = sources_name,
                exp_files = exps_name,
                expected_failure = tests[name]["disabled"],
                extension = extension,
                tags = tags + extra_tags,
                size = "small",
            )

            exp_tests = [validate_exp]

        filecheck_test(
            name = filecheck,
            srcs = sources_name,
            extension = extension,
            tags = tags + extra_tags,
            size = "small",
        )

        if tests[name]["too_slow"]:
            too_slow_tests.append(test_name)
            too_slow_tests.extend(exp_tests)
        else:
            oracle_tests.append(test_name)
            validate_tests.extend(exp_tests)
            filecheck_tests.append(filecheck)

    native.test_suite(
        name = suite_name,
        tests = oracle_tests + validate_tests + filecheck_tests,
        visibility = ["//visibility:public"],
    )

    native.test_suite(
        name = "{}_oracle".format(suite_name),
        tests = oracle_tests,
        visibility = ["//visibility:public"],
    )

    native.test_suite(
        name = "{}_validate_exp".format(suite_name),
        tests = validate_tests,
        visibility = ["//visibility:public"],
    )

    native.test_suite(
        name = "{}_filecheck".format(suite_name),
        tests = filecheck_tests,
        visibility = ["//visibility:public"],
    )

    native.test_suite(
        name = "{}_too_slow".format(suite_name),
        tests = too_slow_tests,
        tags = ["manual"],
        visibility = ["//visibility:public"],
    )

RubyOracle = provider(
    fields = {
        "log": "The log of oracle generation",
        "stdout": "The stdout from ruby",
        "stderr": "The stderr from ruby",
        "exit": "The exit code from ruby",
    },
)

def _build_ruby_oracle_impl(ctx):
    outputs = [ctx.outputs.log, ctx.outputs.stdout, ctx.outputs.stderr, ctx.outputs.exit]

    ctx.actions.run_shell(
        command = """
        if ! "{generate_out_file}" "{stdout}" "{stderr}" "{exit}" "{test}" > "{log}"; then
            cat {log}
            exit 1
        fi
        """.format(
            generate_out_file = ctx.executable._generate_out_file.path,
            log = ctx.outputs.log.path,
            stdout = ctx.outputs.stdout.path,
            stderr = ctx.outputs.stderr.path,
            exit = ctx.outputs.exit.path,
            test = ctx.file.test.path,
        ),
        tools = ctx.files._generate_out_file,
        inputs = ctx.files.srcs,
        outputs = outputs,
        progress_message = "Ruby {}".format(ctx.attr.prefix),
    )

    runfiles = ctx.runfiles(ctx.files._generate_out_file + ctx.files.srcs)

    return [
        DefaultInfo(files = depset(outputs), runfiles = runfiles),
        RubyOracle(
            log = ctx.outputs.log,
            stdout = ctx.outputs.stdout,
            stderr = ctx.outputs.stderr,
            exit = ctx.outputs.exit,
        ),
    ]

build_ruby_oracle = rule(
    implementation = _build_ruby_oracle_impl,
    attrs = {
        "prefix": attr.string(
            mandatory = True,
        ),
        "test": attr.label(
            allow_single_file = True,
        ),
        "srcs": attr.label(
            allow_files = True,
        ),
        "log": attr.output(
            mandatory = True,
        ),
        "stdout": attr.output(
            mandatory = True,
        ),
        "stderr": attr.output(
            mandatory = True,
        ),
        "exit": attr.output(
            mandatory = True,
        ),
        "_generate_out_file": attr.label(
            cfg = "target",
            default = "//test:generate_out_file",
            executable = True,
        ),
    },
    provides = [DefaultInfo, RubyOracle],
)

SorbetOutput = provider(
    fields = {
        "build": "The output directory for test artifacts",
        "log": "The build log",
        "stdout": "The stdout of the compiler",
        "exit": "The exit code of the compiler",
    },
)

def _build_extension_impl(ctx):
    sources = [file.path for file in ctx.files.srcs]

    outputs = [ctx.outputs.out, ctx.outputs.log, ctx.outputs.stdout, ctx.outputs.exit]

    ctx.actions.run_shell(
        command = """
        if ! "{build_extension}" "{output}" "{stdout}" "{exit}" {inputs} 2>&1 > "{log}" 2>&1 ; then
          cat "{log}"
          exit 1
        fi
        """.format(
            build_extension = ctx.executable._build_extension.path,
            output = ctx.outputs.out.path,
            inputs = " ".join([file.path for file in ctx.files.srcs]),
            log = ctx.outputs.log.path,
            stdout = ctx.outputs.stdout.path,
            exit = ctx.outputs.exit.path,
        ),
        tools = ctx.files._build_extension,
        inputs = ctx.files.srcs,
        outputs = outputs,
        progress_message = "Sorbet {}".format(ctx.attr.prefix),
    )

    runfiles = ctx.runfiles(ctx.files._build_extension + ctx.files.srcs)

    return [
        DefaultInfo(files = depset(outputs), runfiles = runfiles),
        SorbetOutput(build = ctx.outputs.out, log = ctx.outputs.log, stdout = ctx.outputs.stdout, exit = ctx.outputs.exit),
    ]

build_extension = rule(
    implementation = _build_extension_impl,
    attrs = {
        "prefix": attr.string(
            mandatory = True,
        ),
        "srcs": attr.label(
        ),
        "out": attr.output(
            mandatory = True,
        ),
        "log": attr.output(
            mandatory = True,
        ),
        "stdout": attr.output(
            mandatory = True,
        ),
        "exit": attr.output(
            mandatory = True,
        ),
        "_build_extension": attr.label(
            cfg = "target",
            default = "//test:build_extension",
            executable = True,
        ),
    },
    provides = [DefaultInfo, SorbetOutput],
)

def _validate_exp_test_impl(ctx):
    output = ctx.attr.extension[SorbetOutput]

    validate_exp = ctx.attr._validate_exp[DefaultInfo]

    runfiles = ctx.runfiles(
        ctx.files.srcs + ctx.files.exp_files + [output.stdout, output.build, output.log],
    )

    runfiles = runfiles.merge(ctx.attr._validate_exp[DefaultInfo].default_runfiles)
    runfiles = runfiles.merge(ctx.attr._llvm_diff[DefaultInfo].default_runfiles)

    ctx.actions.write(
        ctx.outputs.executable,
        content = """
        cat "{build_log}"

        cat "{sorbet_log}"

        {validate_exp} --build_dir="{build_dir}" --expected_failure="{expected_failure}" {sources}
        """.format(
            build_log = output.log.short_path,
            sorbet_log = output.stdout.short_path,
            validate_exp = ctx.executable._validate_exp.short_path,
            build_dir = output.build.short_path,
            sources = " ".join([file.short_path for file in ctx.files.srcs]),
            expected_failure = ctx.attr.expected_failure,
        ),
        is_executable = True,
    )

    return [DefaultInfo(runfiles = runfiles)]

validate_exp_test = rule(
    test = True,
    implementation = _validate_exp_test_impl,
    attrs = {
        "srcs": attr.label(
            mandatory = True,
        ),
        "exp_files": attr.label(
            mandatory = True,
        ),
        "expected_failure": attr.bool(
            default = False,
        ),
        "extension": attr.label(
            mandatory = True,
            providers = [SorbetOutput],
        ),
        "_validate_exp": attr.label(
            cfg = "host",
            default = "//test:validate_exp",
            executable = True,
        ),
        "_llvm_diff": attr.label(
            default = "//test:llvm-diff",
        ),
    },
)

def _filecheck_test_impl(ctx):
    output = ctx.attr.extension[SorbetOutput]

    filecheck = ctx.attr._filecheck[DefaultInfo]

    runfiles = ctx.runfiles(
        ctx.files.srcs + [output.stdout, output.build, output.log],
    )

    runfiles = runfiles.merge(ctx.attr._filecheck[DefaultInfo].default_runfiles)

    ctx.actions.write(
        ctx.outputs.executable,
        content = """
        cat "{build_log}"

        cat "{sorbet_log}"

        {filecheck} --build_dir="{build_dir}" {sources}
        """.format(
            build_log = output.log.short_path,
            sorbet_log = output.stdout.short_path,
            filecheck = ctx.executable._filecheck.short_path,
            build_dir = output.build.short_path,
            sources = " ".join([file.short_path for file in ctx.files.srcs]),
        ),
        is_executable = True,
    )

    return [DefaultInfo(runfiles = runfiles)]

filecheck_test = rule(
    test = True,
    implementation = _filecheck_test_impl,
    attrs = {
        "srcs": attr.label(
            mandatory = True,
        ),
        "extension": attr.label(
            mandatory = True,
            providers = [SorbetOutput],
        ),
        "_filecheck": attr.label(
            cfg = "host",
            default = "//test:filecheck",
            executable = True,
        ),
    },
)

def _validate_sorbet_output_test_impl(ctx):
    sources = [file.short_path for file in ctx.files.srcs]

    build = ctx.attr.build[SorbetOutput]
    oracle = ctx.attr.oracle[RubyOracle]

    ctx.actions.write(
        ctx.outputs.executable,
        content = """
        cat "{build_log}"

        cat "{sorbet_log}"

        cat "{oracle_log}"

        {test_corpus_runner} \\
          --expected_output="{expected_outfile}" \\
          --expected_err="{expected_errfile}" \\
          --expected_exit_code="{expected_exitfile}" \\
          --build_dir="{build_dir}" \\
          --ruby="{ruby}" \\
          --expect_fail="{expect_fail}" \\
          --sorbet_exit="{sorbet_exitfile}" \\
          --sorbet_out="{sorbet_out}" \\
          {sources} 2>&1
        """.format(
            build_log = build.log.short_path,
            sorbet_log = build.stdout.short_path,
            oracle_log = oracle.log.short_path,
            test_corpus_runner = ctx.executable._test_corpus_runner.short_path,
            expected_outfile = oracle.stdout.short_path,
            expected_errfile = oracle.stderr.short_path,
            expected_exitfile = oracle.exit.short_path,
            build_dir = build.build.short_path,
            ruby = ctx.executable._ruby.short_path,
            expect_fail = ctx.attr.expected_failure,
            sorbet_exitfile = build.exit.short_path,
            sorbet_out = build.stdout.short_path,
            sources = " ".join(sources),
        ),
        is_executable = True,
    )

    runfiles = ctx.runfiles(files = [ctx.file._patch_require] + ctx.files.build + ctx.files.oracle + ctx.files._sorbet_runtime +
                                    ctx.files.srcs)
    runfiles = runfiles.merge(ctx.attr._test_corpus_runner[DefaultInfo].default_runfiles)
    runfiles = runfiles.merge(ctx.attr._ruby[DefaultInfo].default_runfiles)

    return [DefaultInfo(runfiles = runfiles)]

validate_sorbet_output_test = rule(
    test = True,
    implementation = _validate_sorbet_output_test_impl,
    attrs = {
        "srcs": attr.label(
        ),
        "build": attr.label(
            providers = [SorbetOutput],
        ),
        "oracle": attr.label(
            providers = [RubyOracle],
        ),
        "expected_failure": attr.bool(
            default = False,
        ),
        "_test_corpus_runner": attr.label(
            cfg = "target",
            default = "//test:test_corpus_runner",
            executable = True,
        ),
        "_ruby": attr.label(
            cfg = "target",
            default = "@sorbet_ruby_2_7_for_compiler//:ruby",
            executable = True,
        ),
        "_sorbet_runtime": attr.label(
            default = "//gems/sorbet-runtime",
        ),
        "_patch_require": attr.label(
            default = ":patch_require.rb",
            allow_single_file = True,
        ),
    },
    provides = [DefaultInfo],
)
