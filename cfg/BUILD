cc_library(
    name = "cfg",
    srcs = glob(
        ["*.cc"],
        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
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
        "cfg",
        "//ast/desugar",
        "//namer",
        "@cxxopts//:cxxopts",
        "@spdlog//:spdlog",
    ],
)
