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
    copts = ["--std=c++14"],
    linkopts = select({
        "//tools/config:linux": ["-lm"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "@concurrentqueue",
        "@spdlog",
    ],
)
