cc_library(
    name = "statsd",
    srcs = select({
        "//tools/config:webasm": ["statsd-emscripten.cc"],
        "//conditions:default": ["statsd.cc"],
    }),
    hdrs = [
        "statsd.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "//core",
    ] + select({
        "//tools/config:webasm": [],
        "//conditions:default": ["@statsd"],
    }),
)
