genrule(
    name = "ragel_lexer",
    srcs = ["cc/lexer.rl"],
    outs = ["cc/lexer.cc"],
    cmd = "ragel -o $@ -C $(location cc/lexer.rl)",
)

genrule(
    name = "bison_parser",
    srcs = [
        "cc/grammars/typedruby24.ypp",
    ],
    outs = [
        "cc/grammars/typedruby24.cc",
        "cc/grammars/typedruby24.hh",
        "cc/grammars/stack.hh",
    ],
    cmd = "bison --defines=$(location cc/grammars/typedruby24.hh) -o $(location cc/grammars/typedruby24.cc) $(location cc/grammars/typedruby24.ypp)",
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
