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
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//cfg",
        "//common",
        "//core",
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
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
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
