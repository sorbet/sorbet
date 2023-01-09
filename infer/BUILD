cc_library(
    name = "infer",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = [
            "*_test.cc",
            "flycheck_*",
        ],
    ),
    hdrs = glob(["*.h"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//cfg",
        "//common",
        "//core",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_test(
    name = "infer_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "infer",
        "//ast/desugar",
        "//cfg",
        "//class_flatten",
        "//local_vars",
        "//namer",
        "//resolver",
        "//rewriter",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)
