cc_library(
    name = "Rewriters",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/Core",
        "//compiler/Names",
        "@com_stripe_ruby_typer//ast",
        "@com_stripe_ruby_typer//ast/treemap",
        "@com_stripe_ruby_typer//core",
        "@spdlog",
    ],
)
