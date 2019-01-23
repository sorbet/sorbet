load("@io_bazel_rules_ragel//ragel:ragel.bzl", "ragel")

ragel(
    name = "ragel_lexer",
    src = "cc/lexer.rl",
    language = "c++",
)

genrule(
    name = "bison_parser",
    srcs = [
        "cc/grammars/typedruby.ypp",
    ],
    outs = [
        "cc/grammars/typedruby.cc",
        "cc/grammars/typedruby.hh",
        "cc/grammars/stack.hh",
    ],
    cmd = "PATH=/usr/local/bin:/usr/local/opt/bison/bin:/usr/bin:/bin bison --defines=$(location cc/grammars/typedruby.hh) -o $(location cc/grammars/typedruby.cc) $(location cc/grammars/typedruby.ypp)",
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
        ":bison_parser",
    ],
    hdrs = glob(["include/**/*.hh"]),
    copts = [
        "-Wno-unused-const-variable",
        "-I$(GENDIR)/external/parser/cc",
        "-I$(GENDIR)/external/parser/cc/grammars",
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
)

exports_files(["codegen/diagnostics.rb"])
