cc_library(
    name = "LLVMIREmitter",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    hdrs = ["LLVMIREmitter.h"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/Errors",
        "//compiler/IRHelpers",
        "//compiler/Names",
        "@com_stripe_ruby_typer//ast",
        "@com_stripe_ruby_typer//cfg",
        "@org_llvm",
        "@spdlog",
    ],
)
