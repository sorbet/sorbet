cc_library(
    name = "linker",
    srcs = [
        "linker.cc",
        "linker.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//llvm/payload",
        "@org_llvm_darwin",
        "@spdlog",
    ],
)
