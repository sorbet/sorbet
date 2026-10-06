"""Connect the foreign-built GNU M4 installation to rules_m4."""

# When changing this adapter, compare with _m4_toolchain_info in
# rules_m4's m4/internal/toolchain.bzl:
# https://github.com/jmillikin/rules_m4/blob/v0.2.1/m4/internal/toolchain.bzl
# That rule supplies the executable and its runfiles to toolchain consumers.
# This launcher adds the foreign installation to those runfiles and forwards
# arguments to its bin/m4, including when invoked from Bison's runfiles tree.
def _m4_launcher_impl(ctx):
    install = ctx.file.install
    executable = ctx.outputs.executable
    ctx.actions.write(
        executable,
        "\n".join([
            "#!/bin/sh",
            "set -eu",
            'runfiles="${RUNFILES_DIR:-$0.runfiles}"',
            'exec "$runfiles/%s/%s/bin/m4" "$@"' % (ctx.workspace_name, install.short_path),
            "",
        ]),
        is_executable = True,
    )
    return [DefaultInfo(
        executable = executable,
        runfiles = ctx.runfiles(files = [executable, install]),
    )]

m4_launcher = rule(
    implementation = _m4_launcher_impl,
    attrs = {
        "install": attr.label(
            allow_single_file = True,
            cfg = "exec",
            mandatory = True,
        ),
    },
    executable = True,
)
