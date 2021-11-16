cc_library(
    name = "minimize",
    srcs = [
        "minimize.cc",
    ],
    hdrs = ["minimize.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "//main/options",
        "@spdlog",
    ],
)
