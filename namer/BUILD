cc_library(
    name = "namer",
    srcs = [
        "namer.cc",
        "namer.h",
    ],
    hdrs = [
        "namer.h",
    ],
    deps = [
        "//ast",
        "//parser",
        "//ast/desugar",
    ],
)

cc_binary(
    name = "namer_print",
    srcs = [
        "main/main.cc",
    ],
    linkstatic = 1,
    deps = [
        "namer",
        "@cxxopts//:cxxopts",
        "@spdlog//:spdlog",
    ],
)

cc_test(
    name = "namer_test",
    srcs = [
        "namer-test.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = 1,
    deps = [
        "namer",
        "//ast/desugar",
        "//parser",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@spdlog//:spdlog",
    ],
)
