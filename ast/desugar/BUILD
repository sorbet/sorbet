cc_library(
    name = "desugar",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    deps = [
        "//ast",
        "//parser",
    ],
)

cc_binary(
    name = "desugar_ast",
    srcs = [
        "main/main.cc",
    ],
    linkstatic = 1,
    deps = [
        "//ast/desugar",
        "@cxxopts//:cxxopts",
        "@spdlog//:spdlog",
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
    deps = [
        "//ast/desugar",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@spdlog//:spdlog",
    ],
)
