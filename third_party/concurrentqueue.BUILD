cc_library(
    name = "concurrentqueue",
    srcs = [],
    hdrs = [
        "blockingconcurrentqueue.h",
        "concurrentqueue.h",
    ],
    visibility = ["//visibility:public"],
    linkstatic = select({
            "@com_stripe_ruby_typer//tools/config:linkshared": 0,
            "//conditions:default": 1,
        }),
)
