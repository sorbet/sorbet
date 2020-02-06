# vim: ft=bzl sw=4 ts=4 et

load("@com_stripe_sorbet_llvm//third_party/ruby:build-ruby.bzl", "build_ruby", "ruby_archive", "ruby_binary", "ruby_headers", "ruby_internal_headers")

filegroup(
    name = "source",
    srcs = glob(["**/*"]),
    visibility = ["//visibility:private"],
)

build_ruby(
    name = "ruby-dist",
    src = ":source",
    bundler = "@bundler_stripe//file",
    copts = [],
    linkopts = [],
    visibility = ["//visibility:private"],
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

ruby_headers(
    name = "headers",
    ruby = ":ruby-dist",
    visibility = ["//visibility:public"],
)

ruby_internal_headers(
    name = "headers-internal",
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
