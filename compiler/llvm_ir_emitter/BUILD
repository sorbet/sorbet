cc_library(
    name = "llvm_ir_emitter",
    srcs = [
        "llvm_ir_emitter.cc",
        "llvm_ir_emitter.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "@com_stripe_ruby_typer//ast",
        "@com_stripe_ruby_typer//cfg",
        "@org_llvm_darwin",
        "@spdlog",
    ],
)
