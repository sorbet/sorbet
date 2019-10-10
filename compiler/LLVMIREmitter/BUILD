cc_library(
    name = "LLVMIREmitter",
    srcs = [
        "LLVMIREmitter.cc",
        "LLVMIREmitter.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/Names",
        "@com_stripe_ruby_typer//ast",
        "@com_stripe_ruby_typer//cfg",
        "@org_llvm_darwin",
        "@spdlog",
    ],
)
