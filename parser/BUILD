package(default_visibility = ["//visibility:public"])

cc_library(
    name = "parser",
    srcs = [
        "Builder.cc",
        "Builder.h",
        "Node.cc",
        "Node_gen.h",
        "Result.cc",
    ],
    hdrs = [
        "Node.h",
        "Result.h",
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

genrule(
    name = "generate_node_h",
    outs = ["Node_gen.h"],
    cmd = "$(location :generate_ast) > $@",
    tools = [":generate_ast"],
)

cc_test(
    name = "parser_test",
    srcs = [
        "parser_test.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    deps = [
        ":parser",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@spdlog//:spdlog",
    ],
)
