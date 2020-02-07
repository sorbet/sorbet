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
    configure_flags = [
        "--enable-shared",
        "--sysconfdir=/etc",
        "--disable-maintainer-mode",
        "--disable-dependency-tracking",
    ],
    copts = [
        "-D_FORTIFY_SOURCE=2",
        "-Wdate-time",
        "-g",
        "-O2",
        "-fstack-protector-strong",
        "-Wformat",
        "-Werror=format-security",

        # The MacOS CROSSTOOL in bazel defines _FORTIFY_SOURCE both on
        # <command line>:1:9: and <built-in>:355:9: so sadly we turn them all off
        "-Wno-macro-redefined",
    ],
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:darwin": [],
        "@com_stripe_ruby_typer//tools/config:linux": [
            "-Wl,-Bsymbolic-functions",
            "-Wl,-z,relro",
        ],
    }),
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
