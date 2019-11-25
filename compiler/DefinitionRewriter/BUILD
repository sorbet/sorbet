cc_library(
    name = "DefinitionRewriter",
    srcs = [
        "DefinitionRewriter.cc",
        "DefinitionRewriter.h",
    ],
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
        "@spdlog",
    ],
)
