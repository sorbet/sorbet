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
    ],
    # Bison 3.3.2's bundled gnulib triggers this warning; remove for 3.8.2.
    copts = ["-Wno-implicit-const-int-float-conversion"],
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
