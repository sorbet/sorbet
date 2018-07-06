cc_library(
    name = "lsp",
    srcs = [
        "lsp.cc",
    ],
    hdrs = ["lsp.h"],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//common/kvstore",
        "//core",
        "//core/proto",
        "//core/serialize",
        "//core/statsd",
        "//main/options",
        "//main/pipeline",
        "//payload/binary",
        "//payload/text",
        "@rapidjson",
    ],
)
