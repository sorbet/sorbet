cc_library(
    name = "cfg",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//common",
        "//parser",
    ],
)

cc_binary(
    name = "cfg_ast",
    srcs = [
        "main/main.cc",
    ],
    linkstatic = 1,
    deps = [
        "//ast/desugar",
        "cfg",
        "@cxxopts//:cxxopts",
        "@spdlog//:spdlog",
    ],
)
