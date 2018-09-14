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
        "//common/statsd",
        "//core",
        "//core/proto",
        "//core/serialize",
        "//main/errorqueue",
        "//main/options",
        "//main/pipeline",
        "//payload/binary",
        "//payload/text",
        "@com_google_absl//absl/synchronization",
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
