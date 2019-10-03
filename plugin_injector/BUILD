cc_library(
    name = "plugin_injector",
    srcs = [
        "plugin_injector.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/object_file_emitter",
        "@com_stripe_ruby_typer//ast",
        "@com_stripe_ruby_typer//cfg",
        "@com_stripe_ruby_typer//main/pipeline/semantic_extension:interface",
        "@cxxopts",
    ],
)
