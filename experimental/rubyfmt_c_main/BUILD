cc_binary(
    name = "main",
    srcs = [
        "main.c",
        "rubyfmt.h",
    ],
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:darwin": ["-framework Foundation"],
        "//conditions:default": ["-lcrypt"],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "@rubyfmt//:librubyfmt",
        "@rubyfmt//:rubyfmt_macro_wrappers",
        "@ruby_2_6//:rubyfmt-static-deps",
    ],
)
