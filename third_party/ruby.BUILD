genrule(
    name = "ruby_genrule",
    srcs = glob(["**"]),
    cmd = """
            # find autoconf on mac
            export PATH="/usr/local/bin:$$PATH"

            # copy the ruby source to a temporary directory to prevent concurrent builds causing side-effects
            DIR=$$(mktemp -d $${TMPDIR-/tmp}/tmp.XXXXXXX)
            (cd `dirname $(location Makefile.in)` && cp -fRL . $$DIR)

            # configure and compile
            (cd $$DIR && autoconf && ./configure --with-soname=ruby --without-gmp && make)

            # copy the static lib and generated platform header
            cp $$DIR/libruby-static.a $(location lib/libruby-static.a)
            cp $$(find $$DIR/.ext -path "**/ruby/config.h" -print -quit) $(location .ext/include/ruby/config.h)
    """,
    outs = ["lib/libruby-static.a", ".ext/include/ruby/config.h"],
    local = 1,
)

cc_library(
    name = "ruby",
    srcs = [":ruby_genrule"] + glob(["include/**/*.h"]),
    hdrs = ["include/ruby.h"],
    includes = ["include", ".ext/include"],
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:linux": ["-lcrypt"],
        "@com_stripe_ruby_typer//tools/config:darwin": ["-framework CoreFoundation"],
        "//conditions:default": [],
    }),
    linkstatic = 1,
    visibility = ["//visibility:public"],
)
