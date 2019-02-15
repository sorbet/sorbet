load("@io_bazel_rules_ragel//ragel:ragel.bzl", "ragel")
load("@io_bazel_rules_bison//bison:bison.bzl", "bison")

ragel(
    name = "ragel_lexer",
    src = "cc/lexer.rl",
    language = "c++",
)

bison(
    name = "typedruby_bison",
    src = "cc/grammars/typedruby.ypp",
)

genrule(
    name = "gen_cpp_diagnostics",
    srcs = [
        "codegen/diagnostics.rb",
    ],
    outs = [
        "include/ruby_parser/diagnostic_class.hh",
    ],
    cmd = "$(location codegen/diagnostics.rb) --cpp=$@",
)

cc_library(
    name = "parser",
    srcs = glob(["cc/*.cc"]) + [
        ":gen_cpp_diagnostics",
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
    visibility = ["//visibility:public"],
    linkstatic = select({
            "@com_stripe_ruby_typer//tools/config:linkshared": 0,
            "//conditions:default": 1,
        }),
   deps = [
        "@com_google_absl//absl/strings",
   ],
)

exports_files(["codegen/diagnostics.rb"])
