# vim: ft=bzl sw=4 ts=4 et

load("@com_stripe_ruby_typer//third_party/ruby:build-ruby.bzl", "ruby")

ruby(
    bundler = "@bundler_2_1_4//file",
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:darwin": [
            "-mlinker-version=400",
        ],
        "@com_stripe_ruby_typer//tools/config:linux": [],
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
