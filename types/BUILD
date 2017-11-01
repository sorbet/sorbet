cc_library(
    name = "types",
    srcs = glob(
        ["*.cc"],
        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = [
            "*_test.cc",
            "flycheck_*",
        ],
    ),
    hdrs = glob(["*.h"]),
    visibility = ["//visibility:public"],
    deps = [
        "//cfg",
        "//common",
    ],
)