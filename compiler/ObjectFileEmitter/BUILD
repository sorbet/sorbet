cc_library(
    name = "object_file_emitter",
    srcs = [
        "object_file_emitter.cc",
        "object_file_emitter.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/payload",
        "@com_stripe_ruby_typer//core",
        "@org_llvm_darwin",
        "@spdlog",
    ],
)
