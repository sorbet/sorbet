cc_library(
    name = "statsd",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        exclude = [
            # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
            "**/flycheck_*",
        ],
    ),
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
        "@statsd",
    ],
)
