cc_library(
    name = "cfg",
    srcs = glob(
        ["*.cc"],
        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = glob(["*.h"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "//core",
    ],
)
