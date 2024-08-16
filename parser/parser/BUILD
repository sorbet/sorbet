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
        "@platforms//cpu:wasm32": [],
        "@com_stripe_ruby_typer//tools/config:opt": ["-G1"],
        "//conditions:default": [],
    }),
)

# It is occasionally useful to have traces of parser behavior.  But turning on
# tracing results in ~4% slower parsing, which seems high enough that it's worth
# doing something different.  To avoid behavior differences between debug and
# non-debug builds, we compile the ruby grammar twice, once with tracing built-in
# and once without.
sh_binary(
    name = "munge_grammar",
    srcs = ["munge_grammar.sh"],
)

genrule(
    name = "generate_release_bison_grammar",
    srcs = ["cc/grammars/typedruby.ypp"],
    outs = [
        "typedruby_release.ypp",
    ],
    cmd = "$(location :munge_grammar) $(location cc/grammars/typedruby.ypp) $(location typedruby_release.ypp)",
    tools = [
        ":munge_grammar",
    ],
)

genrule(
    name = "generate_debug_bison_grammar",
    srcs = ["cc/grammars/typedruby.ypp"],
    outs = [
        "typedruby_debug.ypp",
    ],
    cmd = "$(location :munge_grammar) $(location cc/grammars/typedruby.ypp) $(location typedruby_debug.ypp)",
    tools = [
        ":munge_grammar",
    ],
)

bison(
    name = "typedruby_release_bison",
    src = "typedruby_release.ypp",
    bison_options = [
        "-Wno-empty-rule",
        "-Wno-precedence",
    ],
)

bison(
    name = "typedruby_debug_bison",
    src = "typedruby_debug.ypp",
    bison_options = [
        "-Wno-empty-rule",
        "-Wno-precedence",
        "--debug",
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
        ":typedruby_release_bison",
        ":typedruby_debug_bison",
    ],
    hdrs = glob(["include/**/*.hh"]),
    copts = [
        "-Wno-unused-const-variable",
        "-Wno-unused-but-set-variable",
    ] + select({
        # What we really want here is to say "it's never ok to use -Os, because
        # that will take 2+ minutes to compile." We have to do this to override
        # the .bazelrc options.
        "@com_stripe_ruby_typer//tools/config:dbg": ["-O0"],
        "@com_stripe_ruby_typer//tools/config:opt": ["-O2"],
        # worse to take too much time to compile in development than to run too
        # slow in production, so -O2 by default.
        "//conditions:default": ["-O2"],
    }),
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
