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
    visibility = ["//visibility:public"],
    deps = [],
)
