cc_library(
    name = "parser",
    srcs = [
        "Builder.cc",
        "Builder.h",
        "Dedenter.h",
        "Node.cc",
        "Node.h",
        "Node_gen.cc",
        "Node_gen.h",
        "Result.cc",
        "Result.h",
    ],
    hdrs = [
        "parser.h",
    ],
    visibility = ["//visibility:public"],
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
    visibility = ["//tools:__pkg__"],
)

cc_binary(
    name = "parse_ast",
    srcs = [
        "main/main.cc",
    ],
    linkstatic = 1,
    visibility = ["//tools:__pkg__"],
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
    visibility = ["//tools:__pkg__"],
    deps = [
        ":parser",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@spdlog//:spdlog",
    ],
)
