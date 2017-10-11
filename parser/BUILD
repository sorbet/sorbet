package(default_visibility = ["//visibility:public"])

cc_library(
    name = "parser",
    srcs = [
        "Builder.cc",
        "Builder.h",
        "Node.cc",
        "Node_gen.cc",
        "Node_gen.h",
        "Result.cc",
    ],
    hdrs = [
        "Node.h",
        "Result.h",
        "parser.h"
    ],
    deps = [
        "//ast",
        "@parser//:parser",
    ],
)

cc_binary(
    name = "generate_ast",
    srcs = [
        "tools/generate_ast.cc",
    ],
)

cc_binary(
    name = "parse_ast",
    srcs = [
        "main/main.cc",
    ],
    linkstatic = 1,
    deps = [
        "//parser",
        "@cxxopts//:cxxopts",
        "@spdlog//:spdlog",
    ],
)

genrule(
    name = "generate_node_h",
    outs = [
        "Node_gen.h",
        "Node_gen.cc",
    ],
    cmd = "$(location :generate_ast) $(location Node_gen.h) $(location Node_gen.cc)",
    tools = [":generate_ast"],
)

cc_test(
    name = "parser_test",
    srcs = [
        "parser_test.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = glob([
        "fixtures/*.rb",
        "fixtures/*.rb.exp",
    ]),
    deps = [
        ":parser",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@spdlog//:spdlog",
    ],
)
