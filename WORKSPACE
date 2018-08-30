workspace(name = "com_stripe_ruby_typer")

load(":third_party/externals.bzl", "externals")

externals()

load("@io_bazel_rules_go//go:def.bzl", "go_rules_dependencies", "go_register_toolchains")

go_rules_dependencies()

go_register_toolchains()

BAZEL_VERSION = "0.16.1"

BAZEL_INSTALLER_VERSION_linux_SHA = "17ab70344645359fd4178002f367885e9019ae7507c9c1ade8220f3628383444"

BAZEL_INSTALLER_VERSION_darwin_SHA = "07d5c753738c7186117168770f525b59c39b24103f714be2ffcaadd8e2c53a78"
