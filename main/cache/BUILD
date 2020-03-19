cc_library(
    name = "cache",
    srcs = select({
        "//tools/config:webasm": ["cache-orig.cc"],
        "//conditions:default": ["cache.cc"],
    }),
    hdrs = ["cache.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = select({
        "//tools/config:webasm": [],
        "//conditions:default": [
            "//common/kvstore",
            "//main/options",
            "//main/pipeline",
            "//payload:interface",
            "//version",
        ],
    }),
)

# no-op version of cache
cc_library(
    name = "cache-orig",
    srcs = [
        "cache-orig.cc",
    ],
    hdrs = ["cache.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [],
)
