cc_library(
    name = "cache",
    srcs = [
        "cache.cc",
    ],
    hdrs = ["cache.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common/kvstore",
        "//main/options",
        "//main/pipeline",
        "//payload:interface",
        "//version",
    ],
)
