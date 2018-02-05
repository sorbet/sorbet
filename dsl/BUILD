cc_library(
    name = "dsl",
    srcs = glob(
        ["*.cc"],
        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = glob(["*.h"]),
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//ast/treemap",
        "//common",
        "//core",
    ],
)
