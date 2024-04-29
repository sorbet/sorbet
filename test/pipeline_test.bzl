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
export ASAN_SYMBOLIZER_PATH=`pwd`/external/llvm_toolchain_15_0_7/bin/llvm-symbolizer
set -x
exec {runner} --single_test="{test}" --parser="{parser}"
"""

def _exp_test_impl(ctx):
    ctx.actions.write(
        output = ctx.outputs.executable,
        content = _TEST_SCRIPT.format(
            runner = ctx.executable.runner.short_path,
            test = ctx.file.test.path,
            parser = ctx.attr.parser,
        ),
    )

    runfiles = ctx.runfiles(files = ctx.files.runner + ctx.files.test + ctx.files.data)
    runfiles = runfiles.merge(ctx.attr._llvm_symbolizer[DefaultInfo].default_runfiles)

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
            executable = True,
            cfg = "target",
            allow_files = True,
        ),
        "_llvm_symbolizer": attr.label(
            default = "//test:llvm-symbolizer",
        ),
        "parser": attr.string(default="sorbet"),
    },
)

_GEN_PACKAGE_RUNNER_SCRIPT = """
set -x

{runner} {test_directory}
"""

def _end_to_end_rbi_test_impl(ctx):
    packager_folder = "/packager/"

    rb_file_path = ctx.files.rb_files[0].path
    pos = rb_file_path.find(packager_folder)
    final_dirsep = rb_file_path.find("/", pos + len(packager_folder))
    test_directory = rb_file_path[0:final_dirsep]

    ctx.actions.write(
        output = ctx.outputs.executable,
        content = _GEN_PACKAGE_RUNNER_SCRIPT.format(
            runner = ctx.executable._runner.short_path,
            test_directory = test_directory,
        ),
    )
    runfiles = ctx.runfiles(ctx.files.rb_files)
    runfiles = runfiles.merge(ctx.attr._runner[DefaultInfo].default_runfiles)

    return [DefaultInfo(runfiles = runfiles)]

end_to_end_rbi_test = rule(
    implementation = _end_to_end_rbi_test_impl,
    test = True,
    attrs = {
        "rb_files": attr.label_list(
            allow_files = True,
            mandatory = True,
        ),
        "_runner": attr.label(
            cfg = "target",
            default = "//test:single_package_runner",
            executable = True,
        ),
    },
)

def single_package_rbi_test(name, rb_files):
    end_to_end_rbi_test(
        name = name,
        rb_files = rb_files,
        # This is to get the test to run on the rbi-gen build job, because I
        # can't figure out how to disable the leak sanitizer when running this.
        tags = ["manual"],
        size = "medium",
    )

_TEST_RUNNERS = {
    "PosTests": ":pipeline_test_runner",
    "LSPTests": ":lsp_test_runner",
    "WhitequarkParserTests": ":parser_test_runner",
    "PackagerTests": ":pipeline_test_runner",
}

def pipeline_tests(suite_name, all_paths, test_name_prefix, extra_files = [], tags = [], parser = "sorbet"):
    tests = {}  # test_name-> {"path": String, "prefix": String, "sentinel": String, "isPackage": bool}

    # The packager step needs folder-based steps since folder structure dictates package membership.
    # All immediate subdirs of `/packager/` are individual tests.
    folder_test_dir = "/packager/"

    for path in all_paths:
        if not path.endswith(".rb") and not path.endswith(".rbi"):
            continue

        packager_pos = path.find(folder_test_dir)
        if packager_pos >= 0:
            # This is a folder test.
            final_dirsep = path.find("/", packager_pos + len(folder_test_dir))
            if final_dirsep >= 0:
                test_name = path[0:final_dirsep]
                current = tests.get(test_name)
                if None == current:
                    data = {
                        "path": test_name,
                        "prefix": test_name + "/",
                        "sentinel": test_name,
                        "isMultiFile": False,
                        "isDirectory": True,
                        "disabled": "disabled" in test_name,
                        "isPackage": True,
                    }
                    tests[test_name] = data
                continue

        # This is not a folder test (common case)
        prefix = dropExtension(basename(path).partition("__")[0])

        test_name = dirname(path) + "/" + prefix

        current = tests.get(test_name)
        if None == current:
            data = {
                "path": dirname(path),
                "prefix": "{}/{}".format(dirname(path), prefix),
                "sentinel": path,
                "isMultiFile": "__" in path,
                "isDirectory": False,
                "disabled": "disabled" in path,
                "isPackage": False,
            }
            tests[test_name] = data

    runner = _TEST_RUNNERS.get(test_name_prefix)
    if None == runner:
        fail(msg = "Unknown pipeline test type: {}".format(test_name_prefix))

    enabled_tests = []
    enabled_packager_tests = []
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
            if tests[name]["isPackage"]:
                enabled_packager_tests.append(test_name)

        data = []
        data += extra_files
        if tests[name]["isDirectory"]:
            data += native.glob(["{}**/*".format(prefix)])
        elif tests[name]["isMultiFile"]:
            data += native.glob(["{}*".format(prefix)])
        else:
            data += [sentinel]
            data += native.glob(["{}.*.exp".format(prefix)])
            data += native.glob(["{}.*.rbupdate".format(prefix)])
            data += native.glob(["{}.*.rbiupdate".format(prefix)])
            data += native.glob(["{}.*.rbedited".format(prefix)])
            data += native.glob(["{}.*.minimize.rbi".format(prefix)])

        exp_test(
            name = test_name,
            runner = runner,
            data = data,
            test = sentinel,
            size = "small",
            tags = tags + extra_tags,
            parser = parser,
        )

    native.test_suite(
        name = suite_name,
        tests = enabled_tests,
    )

    if len(enabled_packager_tests) > 0:
        native.test_suite(
            name = "{}_packager".format(suite_name),
            tests = enabled_packager_tests,
        )

    if len(disabled_tests) > 0:
        native.test_suite(
            name = "{}_disabled".format(suite_name),
            tests = disabled_tests,
        )
