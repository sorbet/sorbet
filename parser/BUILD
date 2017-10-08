cc_library(
    name = "parser",
    srcs = [
        "Builder.h",
        "Node_gen.h",
    ] + glob(["*.cc"]),
    hdrs = [
        "Node.h",
        "Result.h",
    ],
    copts = [
        "-I$(GENDIR)/parser/",
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
)

genrule(
    name = "generate_node_h",
    outs = ["Node_gen.h"],
    cmd = "$(location :generate_ast) > $@",
    tools = [":generate_ast"],
)
