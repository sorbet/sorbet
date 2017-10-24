cc_library(
    name = "ast",
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
        "ast.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "@spdlog//:spdlog",
    ],
)

cc_test(
    name = "ast_test",
    srcs = glob(["*_test.cc"]),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":ast",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)
