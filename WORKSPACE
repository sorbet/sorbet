workspace(name = "com_stripe_sorbet_llvm")

load("//third_party:externals.bzl", "sorbet_llvm_externals")

sorbet_llvm_externals()

load("@com_stripe_ruby_typer//third_party:externals.bzl", "register_sorbet_dependencies")

register_sorbet_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain",
    absolute_paths = True,
    llvm_mirror_prefixes = [
        "https://sorbet-deps.s3-us-west-2.amazonaws.com/",
        "https://artifactory-content.stripe.build/artifactory/github-archives/llvm/llvm-project/releases/download/llvmorg-",
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-",
    ],
    llvm_version = "9.0.0",
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains()

load("@rules_ragel//ragel:ragel.bzl", "ragel_register_toolchains")

ragel_register_toolchains()

load("@rules_m4//m4:m4.bzl", "m4_register_toolchains")

m4_register_toolchains()

load("@rules_bison//bison:bison.bzl", "bison_register_toolchains")

bison_register_toolchains()

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

BAZEL_INSTALLER_VERSION_linux_SHA = "6dfcd3b3d7a8811f53a1776c285cf533bc12c33d1eaf49b6105e9699df26ef57"

BAZEL_INSTALLER_VERSION_darwin_SHA = "8a0238c126d086f3641efda177fa8fc1e85ba09c2af10c0977f14b202a9e7a5a"
