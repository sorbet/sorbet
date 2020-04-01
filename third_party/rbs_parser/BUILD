load("@rules_ragel//ragel:ragel.bzl", "ragel")
load("@rules_bison//bison:bison.bzl", "bison")

ragel(
    name = "RBSRagel",
    src = "src/Lexer.rl",
    language = "c++",
)

genrule(
    name = "RBSBison",
    srcs = ["src/Parser.ypp"],
    outs = [
        "src/Parser.cc",
        "src/Parser.hh",
        "src/location.hh",
    ],
    cmd = "M4=$(M4) $(BISON) -o $(location src/Parser.cc) $<",
    toolchains = [
        "@rules_bison//bison:current_bison_toolchain",
        "@rules_m4//m4:current_m4_toolchain",
    ],
)

cc_library(
    name = "rbs_parser",
    srcs = [
        ":RBSBison",
        ":RBSRagel",
    ],
    hdrs = glob(["src/*.hh"]),
    copts = [
        "-Wno-unused-const-variable",
    ],
    includes = ["src"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)
