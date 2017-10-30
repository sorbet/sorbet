cc_library(
    name = "infer",
    srcs = glob(
        ["*.cc"],
        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = glob(["*.h"]),
    visibility = ["//visibility:public"],
    deps = [
        "//cfg",
        "//common",
    ],
)
