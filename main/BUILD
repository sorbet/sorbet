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
        "//typer",
        "@cxxopts//:cxxopts",
        "@spdlog//:spdlog",
    ],
)
