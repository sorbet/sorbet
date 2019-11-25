cc_library(
    name = "IREmitter",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    hdrs = ["IREmitter.h"],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/Errors",
        "//compiler/IRHelpers",
        "//compiler/Names",
        "@com_stripe_ruby_typer//ast",
        "@com_stripe_ruby_typer//cfg",
        "@spdlog",
    ],
)
