cc_library(
    name = "treemap",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        exclude = [
            "*_test.cc",

            # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
            "flycheck_*",
        ],
    ),
    hdrs = [
        "treemap.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//common",
        "//core",
    ],
)
