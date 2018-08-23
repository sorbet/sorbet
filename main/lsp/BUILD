cc_library(
    name = "lsp",
    srcs = glob([
        "*.cc",
        "requests/*.cc",
    ]),
    hdrs = [
        "DefLocSaver.h",
        "lsp.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//common/kvstore",
        "//core",
        "//core/proto",
        "//core/serialize",
        "//core/statsd",
        "//main/errorqueue",
        "//main/options",
        "//main/pipeline",
        "//payload/binary",
        "//payload/text",
        "@rapidjson",
    ],
)

cc_test(
    name = "lsp_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    copts = ["-Iexternal/gtest/include"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)
