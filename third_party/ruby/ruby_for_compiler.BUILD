# vim: ft=bzl sw=4 ts=4 et

load("@com_stripe_ruby_typer//third_party/ruby:build-ruby.bzl", "ruby")

ruby(
    append_srcs = [
        "@com_stripe_ruby_typer//compiler/ruby-static-exports:vm_append_files",
        "@com_stripe_ruby_typer//compiler/IREmitter/Payload/patches:vm_append_files",
    ],
    configure_flags = [
        "--enable-shared",
        "--sysconfdir=/etc",
        "--localstatedir=/var",
        "--disable-maintainer-mode",
        "--disable-dependency-tracking",
        "--disable-jit-support",
        "--disable-install-doc",
    ] + select({
        # Enforce that we don't need Ruby to build in release builds.
        # (In non-release builds, we allow for an available system Ruby to
        # speed up the build.)
        "@com_stripe_ruby_typer//tools/config:release": ["--with-baseruby=no"],
        "//conditions:default": [],
    }),
    copts = [
        "-g",
        "-fstack-protector-strong",
        "-Wformat",
        "-Werror=format-security",
        # The Ruby build is very noisy without this flag.
        "-Wno-compound-token-split-by-macro",
    ] + select({
        "@com_stripe_ruby_typer//tools/config:dbg": [],
        "//conditions:default": ["-O2"],
    }),
    cppopts = [
        "-Wdate-time",
        "-D_FORTIFY_SOURCE=2",
    ],
    extra_srcs = [
        "@com_stripe_ruby_typer//sorbet_version:sorbet_version_srcs",
        "@com_stripe_ruby_typer//compiler/IREmitter/Payload:vm_payload_srcs",
    ],
    gems = [
        "@bundler_stripe//file",
    ],
    linkopts = select({
        "@platforms//os:linux": [
            "-Wl,-Bsymbolic-functions",
            "-Wl,-z,relro",
            "-Wl,-z,noexecstack",
        ],
        "@platforms//os:osx": [
            "-mlinker-version=400",
        ],
        "//conditions:default": [],
    }),
    rubygems = "@rubygems_update_stripe//file",
    deps = select({
        "@platforms//os:osx": [
            "@system_ssl_darwin//:ssl",
            "@system_ssl_darwin//:crypto",
        ],
        "@platforms//os:linux": [
            "@system_ssl_linux//:ssl",
            "@system_ssl_linux//:crypto",
        ],
    }),
)
