load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

filegroup(
    name = "srcs",
    srcs = glob(["**"]),
)

configure_make(
    name = "m4",
    configure_options = [
        # We don't care about translated error messages
        "--disable-nls",
    ],
    lib_source = ":srcs",
    out_binaries = ["m4"],
)

filegroup(
    name = "install",
    srcs = [":m4"],
    output_group = "gen_dir",
    visibility = ["//visibility:public"],
)
