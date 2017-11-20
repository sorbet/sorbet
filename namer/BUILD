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
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//ast/desugar",
        "//core",
        "//parser",
    ],
)

cc_test(
    name = "namer_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    copts = ["-Iexternal/gtest/include"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "namer",
        "//ast/desugar",
        "//parser",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)
