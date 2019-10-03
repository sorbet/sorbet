cc_library(
    name = "llvm_ir_emitter",
    srcs = [
        "llvm_ir_emitter.cc",
        "llvm_ir_emitter.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "@org_llvm_darwin",
        "@spdlog",
        "@com_stripe_ruby_typer//ast",
        "@com_stripe_ruby_typer//cfg",
    ],
)
