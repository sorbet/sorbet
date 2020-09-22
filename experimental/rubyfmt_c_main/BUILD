cc_binary(
    name = "main",
    srcs = [
        "main.c",
        "rubyfmt.h",
    ],
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:darwin": ["-framework Foundation"],
        "//conditions:default": [
            "-lcrypt",
            "-lgmp",
            "-lrt",
        ],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "@ruby_2_6//:rubyfmt-static-deps",
        "@rubyfmt//:librubyfmt",
        "@rubyfmt//:rubyfmt_macro_wrappers",
    ],
)
