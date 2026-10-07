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

# bazel_features is used by rules_cc and toolchains_llvm to detect Bazel version
# capabilities. bazel_features_deps() sets up the @bazel_features_version repository
# which is needed by bazel_features internally.
load("@bazel_features//:deps.bzl", "bazel_features_deps")

bazel_features_deps()

# rules_cc 0.2.14+ requires the @cc_compatibility_proxy repository to be set up
# for WORKSPACE builds. This provides compatibility shims for native cc_* rules.
load("@rules_cc//cc:extensions.bzl", "compatibility_proxy_repo")

compatibility_proxy_repo()

load("@toolchains_llvm//toolchain:rules.bzl", "llvm_toolchain")

# The LLVM 22 ARM64 macOS archive has no x86_64 compiler-rt slice. Bundled
# LLD warns on Intel cross-links; tested int128 helpers resolve via libSystem.
# Keep driver defaults (not -nodefaultlibs, which also removes sanitizer
# runtimes). Recheck the archive and older-macOS coverage on future upgrades.
llvm_toolchain(
    name = "llvm_toolchain_22_1_3",
    absolute_paths = True,
    alternative_llvm_sources = [
        "https://github.com/sorbet/llvm-project/releases/download/llvmorg-{llvm_version}/{basename}",
    ],
    llvm_version = "22.1.3",
)

load("@llvm_toolchain_22_1_3//:toolchains.bzl", "llvm_register_toolchains")

llvm_register_toolchains()

load("@emsdk//:deps.bzl", emsdk_deps = "deps")

emsdk_deps()

load("@rules_python//python:repositories.bzl", "py_repositories", "python_register_toolchains")

py_repositories()

# Register Python's execution toolchain for emsdk's upstream interpreter lookup.
python_register_toolchains(
    name = "emscripten_python",
    ignore_root_user_error = True,
    python_version = "3.11.10",
)

load("@emsdk//:emscripten_deps.bzl", emsdk_emscripten_deps = "emscripten_deps")

emsdk_emscripten_deps(emscripten_version = "4.0.23")

load("@emsdk//:toolchains.bzl", "register_emscripten_toolchains")

register_emscripten_toolchains()

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains(version = "1.20.7")

load("@rules_ragel//ragel:ragel.bzl", "ragel_register_toolchains")

ragel_register_toolchains()

load("@rules_m4//m4:m4.bzl", "m4_register_toolchains")

m4_register_toolchains(
    extra_copts = [
        # M4 1.4.18 and its bundled gnulib trigger the SDK's sprintf
        # deprecation warning. Revisit when upgrading M4/gnulib.
        "-Wno-deprecated-declarations",
        # M4 1.4.18 marks a void fault_handler as pure. Clang ignores the
        # invalid attribute; revisit this suppression on an M4 upgrade.
        "-Wno-ignored-attributes",
    ],
)

load("@rules_bison//bison:bison.bzl", "bison_register_toolchains")

bison_register_toolchains(
    extra_copts = [
        # Bundled gnulib formatting code triggers the macOS SDK's sprintf
        # deprecation warning. Revisit when upgrading Bison's gnulib.
        "-Wno-deprecated-declarations",
        # Bison 3.3.2's bundled gnulib triggers this warning; remove for 3.8.2.
        "-Wno-implicit-const-int-float-conversion",
        # Bison 3.3.2's generated parse-gram.c sets gram_nerrs without using
        # it. Revisit this suppression when upgrading the generator.
        "-Wno-unused-but-set-variable",
    ],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

load("@aspect_bazel_lib//lib:repositories.bzl", "aspect_bazel_lib_dependencies")

aspect_bazel_lib_dependencies()
