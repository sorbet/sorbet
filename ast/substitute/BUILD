cc_library(
    name = "substitute",
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
        "substitute.h",
    ],
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
