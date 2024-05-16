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
    name = "llvm_toolchain_15_0_7",
    absolute_paths = True,
    llvm_mirror_prefixes = [
        "https://artifactory-content.stripe.build/artifactory/github-archives/llvm/llvm-project/releases/download/llvmorg-",
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-",
        "https://artifactory-content.stripe.build/artifactory/github-archives/llvm/llvm-project/releases/download/llvmorg-",
        "https://github.com/sorbet/llvm-project/releases/download/llvmorg-",
    ],
    llvm_version = "15.0.7",
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

BAZEL_INSTALLER_VERSION_LINUX_X86_64_SHA = "c0161a346b9c0d00e6eb3d3e8f9c4dece32f6292520248c5ab2e3527265601c1"

# Bazel for linux-arm64 doesn't have an installer at the moment.
# We have a workaround in `./bazel` to download the binary directly.
BAZEL_INSTALLER_VERSION_LINUX_ARM64_SHA = "5afe973cadc036496cac66f1414ca9be36881423f576db363d83afc9084c0c2f"

BAZEL_INSTALLER_VERSION_DARWIN_X86_64_SHA = "455589bbaedf26e7bdb949288f777492ba1c53d67fd8329bfe066fb988df0e5c"

BAZEL_INSTALLER_VERSION_DARWIN_ARM64_SHA = "c2b5f82dcc1561d25bc05c734a7cc7a5ff58d4e69185f3d6d21b51ddb53b488b"
