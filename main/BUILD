config_setting(
    name = "can-use-jemalloc",
    values = {
        "define": "unsanitized=true",
        "compilation_mode": "opt",
    },
)

cc_binary(
    name = "ruby-typer",
    srcs = [
        "main.cc",
    ],
    copts = ["--std=c++14"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    malloc = select({
        ":can-use-jemalloc": "@jemalloc//:jemalloc",
        "//conditions:default": "@bazel_tools//tools/cpp:malloc",
    }),
    visibility = ["//visibility:public"],
    deps = [
        "realmain",
        "//payload/binary:some",
        "//payload/text:empty",
    ],
)

cc_binary(
    name = "ruby-typer-orig",
    srcs = [
        "main.cc",
    ],
    copts = ["--std=c++14"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "realmain",
        "//payload/binary:empty",
        "//payload/text:some",
    ],
)

cc_library(
    name = "realmain",
    srcs = [
        "realmain.cc",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast/desugar",
        "//ast/substitute",
        "//ast/treemap",
        "//cfg",
        "//cfg/builder",
        "//core",
        "//core/serialize",
        "//dsl",
        "//infer",
        "//namer",
        "//parser",
        "//payload/binary",
        "//payload/text",
        "//resolver",
        "@cxxopts",
    ],
)
