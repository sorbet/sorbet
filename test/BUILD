cc_test(
    name = "hello-test",
    srcs = ["hello-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@spdlog//:spdlog",
        "@cxxopts//:cxxopts",
        "//main:hello-greet",
    ],
)
