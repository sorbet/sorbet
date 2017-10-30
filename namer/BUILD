cc_library(
    name = "namer",
    srcs = glob(
        ["*.cc"],
        exclude = [
            "*_test.cc",
            "flycheck_*",
        ],
    ),
    hdrs = [
        "namer.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//ast/desugar",
        "//parser",
    ],
)

cc_test(
    name = "namer_test",
    size = "small",
    srcs = [
        "namer_test.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = 1,
    visibility = ["//tools:__pkg__"],
    deps = [
        "namer",
        "//ast/desugar",
        "//parser",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@spdlog//:spdlog",
    ],
)
