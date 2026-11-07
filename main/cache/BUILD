cc_library(
    name = "cache",
    srcs = select({
        "@platforms//cpu:wasm32": ["cache-noop.cc"],
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
        "//core/serialize",
    ] + select({
        "@platforms//cpu:wasm32": [],
        "//conditions:default": [
            "//common/concurrency",
            "//main/options",
            "//sorbet_version",
            "@com_google_absl//absl/cleanup",
        ],
    }),
)

# no-op version of cache
cc_library(
    name = "cache-orig",
    srcs = [
        "cache-noop.cc",
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
