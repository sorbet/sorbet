cc_library(
    name = "desugar",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    deps = [
        "//ast",
        "//parser",
    ],
)


cc_binary(
    name = "desugar_ast",
    srcs = [
        "main/main.cc",
    ],
    linkstatic = 1,
    deps = [
        "//ast/desugar",
        "@cxxopts//:cxxopts",
        "@spdlog//:spdlog",
    ],
)