cc_library(
    name = "desugar",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//parser",
    ],
)

cc_test(
    name = "desugar_test",
    srcs = [
        "test/desugar_test.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = glob([
        "fixtures/*.rb",
        "fixtures/*.rb.exp",
    ]),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast/desugar",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@spdlog//:spdlog",
    ],
)
