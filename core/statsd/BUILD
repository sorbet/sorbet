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
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "//core",
        "//core:counters_impl",
        "@statsd",
    ],
)
