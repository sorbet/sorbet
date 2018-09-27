cc_library(
    name = "errorqueue",
    srcs = [
        "ConcurrentErrorQueue.cc",
        "ConcurrentErrorQueue.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common/concurrency",
        "//core",
    ],
)
