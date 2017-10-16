filegroup(
    name = "fixtures",
    srcs = glob([
        "pos/**/*.rb",
        "pos/**/*.rb.exp.*",
    ]),
    data = glob([
        "pos/**/*.rb",
        "pos/**/*.rb.exp.*",
    ]),
)

cc_test(
    name = "test_corpus",
    size = "enormous",
    srcs = [
        "test_corpus.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = glob([
        "pos/**/*.rb",
        "pos/**/*.rb.exp.*",
    ]),
    deps = [
        "//ast/desugar",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@spdlog//:spdlog",
    ],
)
