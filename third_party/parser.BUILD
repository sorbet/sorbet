package(default_visibility = ["//visibility:public"])

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
        "script/mkdiagnostics",
    ],
    outs = [
        "include/ruby_parser/diagnostic_class.hh",
    ],
    cmd = "$(location script/mkdiagnostics) cpp > $@",
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
        "-std=c++14",
        "-I$(GENDIR)/external/parser/cc",
    ],
    includes = [
        "include",
        "include/ruby_parser",
    ],
)
