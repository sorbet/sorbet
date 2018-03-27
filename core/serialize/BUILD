cc_library(
    name = "serialize",
    srcs = ["serialize.cc"],
    hdrs = ["serialize.h"],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//core",
        "@lizard",
    ],
)

cc_test(
    name = "serialize_test",
    size = "small",
    srcs = glob(["test/*.cc"]),
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":serialize",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
