cc_binary(
    name = "ruby-typer",
    srcs = [
        "main.cc",
        "realmain.cc",
    ],
    linkstatic = select({
        "//tools/config:opt": 1,
        "//conditions:default": 0,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast/desugar",
        "//cfg",
        "//infer",
        "//namer",
        "//parser",
        "//rbi:payload",
        "@cxxopts//:cxxopts",
    ],
)

cc_test(
    name = "stdlib_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]) + ["realmain.cc"],
    copts = ["-Iexternal/gtest/include"],
    data = ["//rbi:stdlib"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast/desugar",
        "//cfg",
        "//infer",
        "//namer",
        "//parser",
        "//rbi:payload",
        "@cxxopts//:cxxopts",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)
