load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

filegroup(
    name = "srcs",
    srcs = glob(["**"]),
)

configure_make(
    name = "bison",
    build_data = ["@rules_m4//m4:current_m4_toolchain"],
    configure_options = [
        # We don't care about translated error messages
        "--disable-nls",
        # If we really want colorized bison diagnostics we could try to add
        # this as a build dep
        "--without-libtextstyle",
    ],
    env = {
        "M4": "$(execpath @rules_m4//m4:current_m4_toolchain)",
    },
    lib_source = ":srcs",
    out_binaries = ["bison"],
    out_data_dirs = ["share/bison"],
)

filegroup(
    name = "install",
    srcs = [":bison"],
    output_group = "gen_dir",
    visibility = ["//visibility:public"],
)
