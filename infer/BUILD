cc_library(
    name = "infer",
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
        "//ast",
        "//cfg",
        "//common",
    ],
)

cc_test(
    name = "infer_test",
    srcs = glob([
        "test/*.cc",
    ]),
    copts = ["-Iexternal/gtest/include"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "infer",
        "//ast/desugar",
        "//cfg",
        "//namer",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)
