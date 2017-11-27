cc_library(
    name = "verifier",
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
        "verifier.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//ast/treemap",
        "//common",
        "//core",
    ],
)
