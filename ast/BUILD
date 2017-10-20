package(default_visibility = ["//visibility:public"])

cc_library(
    name = "ast",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        exclude = ["*_test.cc"],
    ),
    hdrs = [
        "ast.h",
    ],
    deps = [
        "//common",
        "@spdlog//:spdlog",
    ],
)

cc_test(
    name = "ast_test",
    srcs = glob(["*_test.cc"]),
    deps = [
        ":ast",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)
