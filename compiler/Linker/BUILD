cc_library(
    name = "Linker",
    srcs = [
        "Linker.cc",
        "Linker.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "@com_stripe_ruby_typer//core",
        "@com_stripe_ruby_typer//common",
        "@spdlog",
    ] + select({
        "@com_stripe_ruby_typer//tools/config:linux": ["@org_llvm_linux"],
        "@com_stripe_ruby_typer//tools/config:darwin": ["@org_llvm_darwin"],
    }),
)
