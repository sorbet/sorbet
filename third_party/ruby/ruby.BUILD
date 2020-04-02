# vim: ft=bzl sw=4 ts=4 et

load("@com_stripe_sorbet_llvm//third_party/ruby:build-ruby.bzl", "ruby")

ruby(
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
    extra_srcs = ["@com_stripe_ruby_typer//sorbet_version:sorbet_version_srcs"],
    gems = [
        "@bundler_stripe//file",
    ],
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:linux": [
            "-Wl,-Bsymbolic-functions",
            "-Wl,-z,relro",
            "-Wl,-z,noexecstack",
        ],
        "//conditions:default": [],
    }),
    rubygems = "@rubygems_update_stripe//file",
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
