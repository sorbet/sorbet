workspace(name = "com_stripe_ruby_typer")

load("//third_party:externals.bzl", "register_sorbet_dependencies")

register_sorbet_dependencies()

load("@com_grail_bazel_compdb//:deps.bzl", "bazel_compdb_deps")

bazel_compdb_deps()

load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain_12_0_0",
    absolute_paths = True,
    llvm_version = "12.0.0",
)

load("@llvm_toolchain_12_0_0//:toolchains.bzl", "llvm_register_toolchains")

llvm_register_toolchains()

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains()

load("@rules_ragel//ragel:ragel.bzl", "ragel_register_toolchains")

ragel_register_toolchains()

load("@rules_m4//m4:m4.bzl", "m4_register_toolchains")

m4_register_toolchains()

load("@rules_bison//bison:bison.bzl", "bison_register_toolchains")

bison_register_toolchains(
    # Clang 12+ introduced this flag. All versions of Bison at time of writing
    # (up to 3.7.6) include code flagged by this warning.
    extra_copts = ["-Wno-implicit-const-int-float-conversion"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

load("@build_bazel_rules_nodejs//:index.bzl", "node_repositories")

node_repositories()

BAZEL_VERSION = "5.2.0"

BAZEL_INSTALLER_VERSION_LINUX_X86_64_SHA = "7d9ef51beab5726c55725fb36675c6fed0518576d3ba51fb4067580ddf7627c4"

BAZEL_INSTALLER_VERSION_DARWIN_X86_64_SHA = "645e7c335efc3207905e98f0c56a598b7cb0282d54d9470e80f38fb698064fb3"

BAZEL_INSTALLER_VERSION_DARWIN_ARM64_SHA = "bc018ee7980cdf1c3f0099ec1568847a1756a3c00f1f9440bca44c26ceb3d90f"
