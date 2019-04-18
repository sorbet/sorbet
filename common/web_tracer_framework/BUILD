cc_library(
    name = "tracing",
    srcs = ["tracing.cc"],
    hdrs = [
        "tracing.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "//core",
    ],
)
