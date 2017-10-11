package(default_visibility = ["//visibility:public"])

cc_library(
    name = "ast",
    srcs = glob(["*.cc"]) + glob(["*.h"]),
    hdrs = [
        "ast.h",
    ],
    deps = [
        "//common",
        "@spdlog//:spdlog",
    ],
)
