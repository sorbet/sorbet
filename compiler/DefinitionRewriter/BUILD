cc_library(
    name = "DefinitionRewriter",
    srcs = [
        "DefinitionRewriter.cc",
        "DefinitionRewriter.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "@com_stripe_ruby_typer//ast",
        "@com_stripe_ruby_typer//ast/treemap",
        "//compiler/Names",
        "@spdlog",
    ],
)
