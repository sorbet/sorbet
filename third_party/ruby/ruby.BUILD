# vim: ft=bzl sw=4 ts=4 et

load("@com_stripe_sorbet_llvm//third_party/ruby:build-ruby.bzl", "ruby")

ruby(
    bundler = "@bundler_stripe//file",
    configure_flags = [
        "--enable-shared",
        "--sysconfdir=/etc",
        "--localstatedir=/var",
        "--disable-maintainer-mode",
        "--disable-dependency-tracking",
    ],
    copts = [
        "-g",
        "-fstack-protector-strong",
        "-Wformat",
        "-Werror=format-security",
    ] + select({
        "@com_stripe_ruby_typer//tools/config:dbg": [],
        "//conditions:default": ["-O2"],
    }),
    cppopts = [
        "-Wdate-time",
        "-D_FORTIFY_SOURCE=2",
    ],
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:linux": [
            "-Wl,-Bsymbolic-functions",
            "-Wl,-z,relro",
            "-Wl,-z,noexecstack",
        ],
        "//conditions:default": [],
    }),
    deps = select({
        "@com_stripe_ruby_typer//tools/config:darwin": [
            "@system_ssl_darwin//:ssl",
            "@system_ssl_darwin//:crypto",
        ],
        "@com_stripe_ruby_typer//tools/config:linux": [
            "@system_ssl_linux//:ssl",
            "@system_ssl_linux//:crypto",
        ],
    }),
)
