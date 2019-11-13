cc_library(
    name = "ObjectFileEmitter",
    srcs = [
        "ObjectFileEmitter.cc",
        "ObjectFileEmitter.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/IRHelpers",
        "@com_stripe_ruby_typer//core",
        select({
            "@com_stripe_ruby_typer//tools/config:linux": "@org_llvm_linux",
            "@com_stripe_ruby_typer//tools/config:darwin": "@org_llvm_darwin",
        }),
        "@spdlog",
    ],
)
