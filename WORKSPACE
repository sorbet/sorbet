workspace(name = "com_stripe_ruby_typer")

load(":third_party/externals.bzl", "externals")

externals()

load("@com_grail_bazel_toolchain//toolchain:configure.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain",
    llvm_version = "7.0.0",
)

load("@io_bazel_rules_go//go:def.bzl", "go_rules_dependencies", "go_register_toolchains")

go_rules_dependencies()

go_register_toolchains()

BAZEL_VERSION = "0.17.2"

BAZEL_INSTALLER_VERSION_linux_SHA = "31fac8b2edcc6d95f1afb21725f604479eb440596e7fc7554fd47e293020ced9"

BAZEL_INSTALLER_VERSION_darwin_SHA = "f5276007719eee0f29904a3a640383cdba99db9fba40b6a3684ca3098ddeb3c7"

