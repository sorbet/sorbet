cc_library(
    name = "opentelemetry",
    srcs = select({
        "//tools/config:webasm": ["opentelemetry-emscripten.cc"],
        "//conditions:default": ["opentelemetry.cc"],
    }),
    hdrs = [
        "opentelemetry.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
    ] + select({
        "//tools/config:webasm": [],
        "//conditions:default": [
            "@opentelemetry//api",
            "@opentelemetry//sdk/src/trace",
        ],
    }),
)
