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
        "//compiler/IRHelpers",
        "@com_stripe_ruby_typer//core",
        "@spdlog",
    ] + select({
        "@com_stripe_ruby_typer//tools/config:linux": ["@org_llvm_linux"],
        "@com_stripe_ruby_typer//tools/config:darwin": ["@org_llvm_darwin"],
    }),
)
