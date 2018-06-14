workspace(name = "com_stripe_ruby_typer")

load(":externals.bzl", "externals")

externals()

load("@io_bazel_rules_go//go:def.bzl", "go_rules_dependencies", "go_register_toolchains")

go_rules_dependencies()

go_register_toolchains()

BAZEL_VERSION = "0.10.0"

# All versions up until 0.15 break some of our dependencies. https://github.com/bazelbuild/bazel/pull/5146

BAZEL_VERSION_SHA = "47e0798caaac4df499bce5fe554a914abd884a855a27085a4473de1d737d9548"
