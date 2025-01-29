cc_library(
    name = "utils",
    srcs = ["Utils.cc"],
    hdrs = ["Utils.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//core",
    ],
)
