cc_binary(
    name = "ruby-typer",
    srcs = ["main.cc"],
    linkstatic = 1,
    deps = [
        "//ast/desugar",
        "//cfg",
        "//namer",
        "//parser",
        "@cxxopts//:cxxopts",
        "@spdlog//:spdlog",
    ],
    visibility = ["//visibility:public"],
)
