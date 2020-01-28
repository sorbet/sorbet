cc_library(
    name = "kvstore",
    srcs = select({
        "//tools/config:webasm": ["KeyValueStore-emscripten.cc"],
        "//conditions:default": ["KeyValueStore.cc"],
    }),
    hdrs = [
        "KeyValueStore.h",
    ],
    linkopts = select({
        "//tools/config:linux": ["-lm"],
        "//conditions:default": [],
    }),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
    ] + select({
        "//tools/config:webasm": [],
        "//conditions:default": ["@lmdb"],
    }),
)
