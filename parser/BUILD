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
        "Parser.cc",
        "diagnostics.cc",
        "parser.h",
    ],
    hdrs = [
        "parser.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//core",
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

genrule(
    name = "generate_node_h",
    outs = [
        "Node_gen.h",
        "Node_gen.cc",
    ],
    cmd = "$(location :generate_ast) $(location Node_gen.h) $(location Node_gen.cc)",
    tools = [":generate_ast"],
)

genrule(
    name = "generate_diagnostics_cc",
    srcs = [
        "tools/generate_diagnostics.rb",
        "@parser//:codegen/diagnostics.rb",
    ],
    outs = [
        "diagnostics.cc",
    ],
    cmd = "ruby $(location tools/generate_diagnostics.rb) $(location @parser//:codegen/diagnostics.rb) $(location diagnostics.cc)",
)

cc_test(
    name = "parser_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    copts = ["-Iexternal/gtest/include"],
    data = glob([
        "fixtures/*.rb",
        "fixtures/*.rb.exp",
    ]),
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":parser",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)
