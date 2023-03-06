cc_library(
    name = "os",
    srcs = [
        "os.h",
        "os.cc",
    ] + select({
        "@platforms//os:osx": ["mac.cc"],
        "//tools/config:webasm": ["emscripten.cc"],
        "//conditions:default": ["linux.cc"],
    }),
    hdrs = [
        "os.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/debugging:symbolize",
        "@spdlog",
    ],
)
