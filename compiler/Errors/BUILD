cc_library(
    name = "Errors",
    srcs = [
        "Errors.h",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "@com_stripe_ruby_typer//core",
    ],
)
