licenses(["notice"])

load("@rules_ragel//ragel:ragel.bzl", "ragel")
load("@rules_bison//bison:bison.bzl", "bison")

ragel(
    name = "ragel_lexer",
    src = "cc/lexer.rl",
    language = "c++",
    ragel_options = select({
        # emscripten builds are usually dealing with small amounts of Ruby source,
        # so they don't need a super-fast parser.
        "@com_stripe_ruby_typer//tools/config:webasm_opt": [],
        "@com_stripe_ruby_typer//tools/config:webasm": [],
        "@com_stripe_ruby_typer//tools/config:opt": ["-G1"],
        "//conditions:default": [],
    }),
)

bison(
    name = "typedruby_bison",
    src = "cc/grammars/typedruby.ypp",
    bison_options = [
        "-Wno-empty-rule",
        "-Wno-precedence",
    ],
)

cc_binary(
    name = "generate_diagnostics",
    srcs = [
        "codegen/generate_diagnostics.cc",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

genrule(
    name = "gen_diagnostics_dclass",
    outs = [
        "include/ruby_parser/diagnostic_class.hh",
    ],
    cmd = "$(location :generate_diagnostics) dclass > $@",
    tools = [
        ":generate_diagnostics",
    ],
)

cc_library(
    name = "parser",
    srcs = glob(["cc/*.cc"]) + [
        ":gen_diagnostics_dclass",
        ":ragel_lexer",
        ":typedruby_bison",
    ],
    hdrs = glob(["include/**/*.hh"]),
    copts = [
        "-Wno-unused-const-variable",
    ],
    includes = [
        "include",
        "include/ruby_parser",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "@com_google_absl//absl/strings",
    ],
)
