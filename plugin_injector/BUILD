cc_library(
    name = "plugin_injector",
    srcs = [
        "plugin_injector.cc",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/Errors",
        "//compiler/IREmitter",
        "//compiler/ObjectFileEmitter",
        "@com_google_absl//absl/cleanup",
        "@com_stripe_ruby_typer//ast",
        "@com_stripe_ruby_typer//cfg",
        "@com_stripe_ruby_typer//common",
        "@com_stripe_ruby_typer//main/options",
        "@com_stripe_ruby_typer//main/pipeline/semantic_extension:interface",
        "@cxxopts",
    ],
)
