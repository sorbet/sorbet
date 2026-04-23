cc_library(
    name = "lz4",
    srcs = [
        "lib/lz4.c",
    ],
    hdrs = [
        "lib/lz4.h",
    ],
    copts = select({
        "@com_stripe_ruby_typer//tools/config:dbg": ["-O2"],
        "//conditions:default": [],
    }),
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)
