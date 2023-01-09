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
        "//parser/parser",
        "@com_google_absl//absl/algorithm:container",
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
    outs = [
        "diagnostics.cc",
    ],
    cmd = "$(location //parser/parser:generate_diagnostics) dclassStrings > $(location diagnostics.cc) && \
           $(location //tools:clang-format) -i $(location diagnostics.cc)",
    tools = [
        "//parser/parser:generate_diagnostics",
        "//tools:clang-format",
    ],
)

cc_test(
    name = "parser_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
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
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)
