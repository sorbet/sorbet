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
        "//sorbet_version",
    ] + select({
        "//tools/config:webasm": [],
        "//conditions:default": [
            "@opentelemetry//api",
            "@opentelemetry//exporters/ostream:ostream_span_exporter",
            "@opentelemetry//sdk/src/trace",
        ],
    }),
)
