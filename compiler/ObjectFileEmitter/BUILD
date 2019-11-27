cc_library(
    name = "ObjectFileEmitter",
    srcs = [
        "ObjectFileEmitter.cc",
        "ObjectFileEmitter.h",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/Linker",
        "//compiler/Passes",
        "//compiler/IREmitter/Payload",
        "@com_stripe_ruby_typer//core",
        "@spdlog",
    ],
)
