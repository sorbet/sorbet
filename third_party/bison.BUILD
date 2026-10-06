load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

filegroup(
    name = "srcs",
    srcs = glob(["**"]),
)

configure_make(
    name = "bison",
    build_data = ["@com_stripe_ruby_typer//third_party/m4:m4"],
    configure_options = [
        # We don't care about translated error messages
        "--disable-nls",
    ],
    # Bison 3.3.2's bundled gnulib triggers this warning; remove for 3.8.2.
    copts = ["-Wno-implicit-const-int-float-conversion"],
    env = {
        "M4": "$(execpath @com_stripe_ruby_typer//third_party/m4:m4)",
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
