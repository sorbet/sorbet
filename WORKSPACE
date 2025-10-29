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

load("@toolchains_llvm//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@toolchains_llvm//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain_16_0_5",
    absolute_paths = True,
    alternative_llvm_sources = [
        "https://github.com/sorbet/llvm-project/releases/download/llvmorg-{llvm_version}/{basename}",
    ],
    llvm_version = "16.0.5",
    # The sysroots are needed for cross-compiling
    sysroot = {
        "": "",
        "darwin-x86_64": "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk",
        "darwin-aarch64": "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk",
    },
)

load("@llvm_toolchain_16_0_5//:toolchains.bzl", "llvm_register_toolchains")

llvm_register_toolchains()

load("@emsdk//:deps.bzl", emsdk_deps = "deps")

emsdk_deps()

load("@emsdk//:emscripten_deps.bzl", emsdk_emscripten_deps = "emscripten_deps")

emsdk_emscripten_deps(emscripten_version = "3.1.59")

load("@emsdk//:toolchains.bzl", "register_emscripten_toolchains")

register_emscripten_toolchains()

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

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

load("@aspect_bazel_lib//lib:repositories.bzl", "aspect_bazel_lib_dependencies")

aspect_bazel_lib_dependencies()
