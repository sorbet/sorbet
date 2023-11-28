workspace(name = "com_stripe_ruby_typer")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//third_party:externals.bzl", "register_sorbet_dependencies")

register_sorbet_dependencies()

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

# We need to explicitly pull in make here for rules_foreign_cc
# to be able to build in CI.
http_archive(
    name = "gnumake_src",
    build_file_content = """\
filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)
""",
    sha256 = "581f4d4e872da74b3941c874215898a7d35802f03732bdccee1d4a7979105d18",
    strip_prefix = "make-4.4",
    urls = ["https://mirror.bazel.build/ftpmirror.gnu.org/gnu/make/make-4.4.tar.gz"],
)

rules_foreign_cc_dependencies()

load("@com_grail_bazel_compdb//:deps.bzl", "bazel_compdb_deps")

bazel_compdb_deps()

load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain_12_0_0",
    absolute_paths = True,
    llvm_mirror_prefixes = [
        "https://sorbet-deps.s3-us-west-2.amazonaws.com/",
        "https://artifactory-content.stripe.build/artifactory/github-archives/llvm/llvm-project/releases/download/llvmorg-",
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-",
    ],
    llvm_version = "12.0.0",
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains(version = "1.20.7")

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

load("@rules_rust//rust:repositories.bzl", "rules_rust_dependencies", "rust_register_toolchains")

rules_rust_dependencies()

rust_register_toolchains(
    edition = "2021",
    versions = [
        "1.58.1",
    ],
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

load("@aspect_bazel_lib//lib:repositories.bzl", "aspect_bazel_lib_dependencies")

aspect_bazel_lib_dependencies()

BAZEL_VERSION = "6.3.2"

BAZEL_INSTALLER_VERSION_LINUX_X86_64_SHA = "f117506267ed148d5f4f9844bcf187c4f111dad7ff4f1a9eb1f4e45331f3f9f0"

BAZEL_INSTALLER_VERSION_LINUX_ARM64_SHA = "9d88a0b206e22cceb4afe0060be7f294b423f5f49b18750fbbd7abd47cea4054"

BAZEL_INSTALLER_VERSION_DARWIN_X86_64_SHA = "b99bb7af239523303c270f27d24c0e5ecea5fa518907f46627a6df3423ac22bd"

BAZEL_INSTALLER_VERSION_DARWIN_ARM64_SHA = "ccc397820fe1bffe8014204dd5110d82e9c09b75462acb0ed73c8920cb3a7b51"
