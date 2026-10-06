"""Connect the foreign-built Bison installation to rules_bison."""

load("@rules_m4//m4:m4.bzl", "M4_TOOLCHAIN_TYPE", "m4_toolchain")

# When changing this adapter, compare with _bison_toolchain_info in
# rules_bison's bison/internal/toolchain.bzl:
# https://github.com/jmillikin/rules_bison/blob/478079b28605a38000eaf83719568d756b3383a0/bison/internal/toolchain.bzl
# Both supply M4 and BISON_PKGDATADIR through runfiles; this launcher uses
# the GNU installation's share/bison directory instead of rules_bison's data/.
def _bison_launcher_impl(ctx):
    m4 = m4_toolchain(ctx)
    install = ctx.file.install
    executable = ctx.outputs.executable
    ctx.actions.write(
        executable,
        "\n".join([
            "#!/bin/sh",
            "set -eu",
            'runfiles="${RUNFILES_DIR:-$0.runfiles}"',
            # M4 is also a launcher and must share Bison's runfiles tree.
            'export RUNFILES_DIR="$runfiles"',
            'export BISON_PKGDATADIR="$runfiles/%s/%s/share/bison"' % (ctx.workspace_name, install.short_path),
            'export M4="$runfiles/%s/%s"' % (ctx.workspace_name, m4.m4_tool.executable.short_path),
            'exec "$runfiles/%s/%s/bin/bison" "$@"' % (ctx.workspace_name, install.short_path),
            "",
        ]),
        is_executable = True,
    )
    return [DefaultInfo(
        executable = executable,
        runfiles = ctx.runfiles(
            files = [executable, install],
            transitive_files = m4.all_files,
        ),
    )]

bison_launcher = rule(
    implementation = _bison_launcher_impl,
    attrs = {
        "install": attr.label(
            allow_single_file = True,
            cfg = "exec",
            mandatory = True,
        ),
    },
    executable = True,
    toolchains = [M4_TOOLCHAIN_TYPE],
)
