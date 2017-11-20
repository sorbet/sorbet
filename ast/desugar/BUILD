cc_library(
    name = "desugar",
    srcs = glob(
        ["*.cc"],

        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = glob(["*.h"]),
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//core",
        "//parser",
    ],
)

cc_test(
    name = "desugar_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    copts = ["-Iexternal/gtest/include"],
    data = glob([
        "fixtures/*.rb",
        "fixtures/*.rb.exp",
    ]),
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast/desugar",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)
