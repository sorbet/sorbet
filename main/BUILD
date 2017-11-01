cc_binary(
    name = "ruby-typer",
    srcs = ["main.cc"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//ast/desugar",
        "//cfg",
        "//infer",
        "//namer",
        "//parser",
        "@cxxopts//:cxxopts",
        "@spdlog//:spdlog",
    ],
)
