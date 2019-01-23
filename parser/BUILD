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
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "@parser",
    ],
)

cc_binary(
    name = "generate_ast",
    srcs = [
        "tools/generate_ast.cc",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
)

genrule(
    name = "generate_node_h",
    outs = [
        "Node_gen.h",
        "Node_gen.cc",
    ],
    cmd = "$(location :generate_ast) $(location Node_gen.h) $(location Node_gen.cc) && \
             $(location //tools:clang-format) -i $(location Node_gen.cc) $(location Node_gen.h)",
    tools = [
        ":generate_ast",
        "//tools:clang-format",
    ],
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
    cmd = "ruby $(location tools/generate_diagnostics.rb) $(location @parser//:codegen/diagnostics.rb) $(location diagnostics.cc) && \
           $(location //tools:clang-format) -i $(location diagnostics.cc)",
    tools = ["//tools:clang-format"],
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
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":parser",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)
