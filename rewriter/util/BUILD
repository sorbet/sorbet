cc_library(
    name = "util",
    srcs = ["Util.cc"],
    hdrs = ["Util.h"],
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
