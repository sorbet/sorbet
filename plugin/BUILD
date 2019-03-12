cc_library(
    name = "plugin",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//ast/treemap",
        "//common",
        "//core",
    ],
)
