cc_library(
    name = "common",
    srcs = glob(
        [
            "**/*.cc",
            "**/*.h",
        ],

        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = [
        "common.h",
        "typecase.h",
    ],
    copts = ["--std=c++14"],
    linkopts = select({
        "//tools/config:linux": ["-lm"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//proto",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/strings",
        "@concurrentqueue",
        "@progressbar",
        "@spdlog",
        "@statsd",
    ],
)
