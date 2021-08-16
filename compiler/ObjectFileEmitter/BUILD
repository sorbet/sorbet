cc_library(
    name = "ObjectFileEmitter",
    srcs = [
        "ObjectFileEmitter.cc",
        "ObjectFileEmitter.h",
    ],
    hdrs = ["ObjectFileEmitter.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/IREmitter/Payload",
        "//compiler/Linker",
        "//compiler/Passes",
        "//core",
        "@spdlog",
    ],
)
