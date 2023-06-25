cc_library(
    name = "sqlitepp",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],

        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = [
        "sqlitepp.h",
    ],
    linkopts = select({
        "@platforms//os:linux": ["-lm"],
        "//conditions:default": [],
    }),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "@sqlite",
    ],
)
