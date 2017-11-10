cc_library(
    name = "core",
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
        "core.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
    ],
)

cc_test(
    name = "core_test",
    size = "small",
    srcs = glob(["test/*.cc"]),
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":core",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)
