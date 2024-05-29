cc_library(
    name = "kvstore",
    srcs = select({
        "@platforms//cpu:wasm32": ["KeyValueStore-emscripten.cc"],
        "//conditions:default": ["KeyValueStore.cc"],
    }),
    hdrs = [
        "KeyValueStore.h",
    ],
    linkopts = select({
        "@platforms//os:linux": ["-lm"],
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
        "@platforms//cpu:wasm32": [],
        "//conditions:default": ["@lmdb"],
    }),
)

cc_test(
    name = "kvstore_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "kvstore",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)
