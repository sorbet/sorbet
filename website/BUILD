load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_file")

sh_binary(
    name = "generate_cli_ref",
    srcs = ["scripts/generate_cli_ref.sh"],
    args = ["$(location //main:sorbet-orig)"],
    data = ["//main:sorbet-orig"],
)

cli_ref_md = "docs/cli-ref.md"

cli_ref_md_gen = "{}.gen".format(cli_ref_md)

genrule(
    name = "generate_cli_ref_md",
    outs = [cli_ref_md_gen],
    cmd = "$(location :generate_cli_ref) $(location //main:sorbet-orig) > $(location {})".format(cli_ref_md_gen),
    tools = [
        ":generate_cli_ref",
        "//main:sorbet-orig",
    ],
)

# Automatically creates a diff_test for this file, called `update_cli_ref_test`
write_source_file(
    name = "update_cli_ref",
    in_file = ":generate_cli_ref_md",
    out_file = cli_ref_md,
)
