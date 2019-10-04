cc_library(
    name = "ObjectFileEmitter",
    srcs = [
        "ObjectFileEmitter.cc",
        "ObjectFileEmitter.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/Payload",
        "@com_stripe_ruby_typer//core",
        "@org_llvm_darwin",
        "@spdlog",
    ],
)
