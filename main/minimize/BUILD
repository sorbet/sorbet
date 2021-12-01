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
        "//ast",
        "//common/concurrency",
        "//common/kvstore",
        "//core",
        "//main/options",
        "//main/pipeline:pipeline-orig",
        "@spdlog",
    ],
)
