def _clang_tool_impl(ctx):
    ctx.actions.write(ctx.outputs.executable,
                      "\n".join(
                          ['#!/bin/sh',
                           'exec $0.runfiles/%s/%s "$@"' % (
                               ctx.workspace_name,
                               ctx.file.tool.short_path),
                           '']),
                      True)

    return [DefaultInfo(runfiles=ctx.runfiles(files=[ctx.file.tool]))]

_clang_tool = rule(
    _clang_tool_impl,
    attrs = {
        "tool": attr.label(
            allow_files = True,
            single_file = True,
            cfg = "host",
        ),
    },
    executable = True,
)

def clang_tool(name):
    native.config_setting(
        name = "darwin",
        values = {"host_cpu": "darwin"},
    )

    native.config_setting(
        name = "linux",
        values = {"host_cpu": "k8"},
    )

    _clang_tool(
        name = name,
        tool = select({
            ":darwin": "@clang_5_0_0_darwin//:bin/" + name,
            ":linux": "@clang_5_0_0_linux//:bin/" + name,
        }),
    )
