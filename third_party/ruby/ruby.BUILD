# vim: ft=bzl sw=4 ts=4 et

load("@com_stripe_ruby_typer//third_party/ruby:build-ruby.bzl", "build_ruby", "ruby_archive", "ruby_binary", "ruby_headers")

filegroup(
    name = "source",
    srcs = glob(["**/*"]),
    visibility = ["//visibility:private"],
)

build_ruby(
    name = "ruby-dist",
    src = ":source",
    ssl = select({
        "@com_stripe_ruby_typer//tools/config:darwin": "@system_ssl_darwin//:ssl",
        "@com_stripe_ruby_typer//tools/config:linux": "@system_ssl_linux//:ssl",
    }),
    crypto = select({
        "@com_stripe_ruby_typer//tools/config:darwin": "@system_ssl_darwin//:crypto",
        "@com_stripe_ruby_typer//tools/config:linux": "@system_ssl_linux//:crypto",
    }),
    visibility = ["//visibility:private"],
)

ruby_headers(
    name = "headers",
    ruby = ":ruby-dist",
    visibility = ["//visibility:public"],
)

ruby_archive(
    name = "ruby.tar.gz",
    ruby = ":ruby-dist",
    visibility = ["//visibility:public"],
)

ruby_binary(
    name = "ruby",
    ruby = ":ruby-dist",
    visibility = ["//visibility:public"],
)

ruby_binary(
    name = "irb",
    ruby = ":ruby-dist",
    visibility = ["//visibility:public"],
)

ruby_binary(
    name = "gem",
    ruby = ":ruby-dist",
    visibility = ["//visibility:public"],
)
