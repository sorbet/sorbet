cc_binary(
    name = "ruby-typer",
    srcs = [
        "main.cc",
    ],
    copts = ["--std=c++14"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "realmain",
        "//payload/binary",
    ],
)

cc_binary(
    name = "ruby-typer-orig",
    srcs = [
        "main.cc",
    ],
    copts = ["--std=c++14"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "realmain",
        "//payload/binary:empty",
    ],
)

cc_library(
    name = "realmain",
    srcs = [
        "realmain.cc",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast/desugar",
        "//ast/treemap",
        "//cfg",
        "//cfg/builder",
        "//core",
        "//core/serialize",
        "//infer",
        "//namer",
        "//parser",
        "//payload/binary:some",
        "//payload/text",
        "@cxxopts//:cxxopts",
    ],
)

cc_test(
    name = "stdlib_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//rbi:stdlib",
        "//test:gerald",
    ],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "realmain",
        "//payload/binary:empty",
        "@cxxopts//:cxxopts",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)
