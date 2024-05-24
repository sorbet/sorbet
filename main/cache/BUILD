cc_library(
    name = "cache",
    srcs = select({
        "@platforms//cpu:wasm32": ["cache-orig.cc"],
        "//conditions:default": ["cache.cc"],
    }),
    hdrs = ["cache.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common/kvstore",
    ] + select({
        "@platforms//cpu:wasm32": [],
        "//conditions:default": [
            "//main/options",
            "//main/pipeline",
            "//payload:interface",
            "//sorbet_version",
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
    deps = [
        "//common/kvstore",
    ],
)
