cc_binary(
    name = "ruby-typer",
    srcs = ["main.cc"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//ast/desugar",
        "//cfg",
        "//namer",
        "//parser",
        "//infer",
        "@cxxopts//:cxxopts",
        "@spdlog//:spdlog",
    ],
)
