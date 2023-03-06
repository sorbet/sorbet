cc_library(
    name = "concurrency",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],

        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = [
        "ConcurrentQueue.h",
        "WorkerPool.h",
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
        "//common/enforce_no_timer",
        "//common/exception",
        "//common/os",
        "@concurrentqueue",
        "@spdlog",
    ],
)
