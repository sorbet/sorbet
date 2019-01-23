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
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "infer",
        "//ast/desugar",
        "//cfg",
        "//dsl",
        "//namer",
        "//resolver",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)
