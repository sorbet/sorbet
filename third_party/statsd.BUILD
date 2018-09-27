cc_library(
    name = "statsd",
    srcs = ["statsd-client.c"],
    hdrs = [
        "statsd-client.h",
    ],
    copts = [],
    visibility = ["//visibility:public"],
    linkstatic = select({
            "@com_stripe_ruby_typer//tools/config:linkshared": 0,
            "//conditions:default": 1,
        }),
)
