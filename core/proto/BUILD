cc_library(
    name = "proto",
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
        "proto.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "//core",
        "//core:counters_impl",
        "//proto",
        "//proto/pay-server",
    ],
)
