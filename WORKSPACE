workspace(name = "com_stripe_ruby_typer")

load("//third_party:externals.bzl", "register_sorbet_dependencies")

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

load("//third_party/gems:rules.bzl", "gemfile_lock_deps")

gemfile_lock_deps(
    name = "gems",

    # TODO: figure out how to discover these labels during the loading phase,
    # rather that listing them out explicitly. `glob` doesn't work here, so
    # we'll need to do something special within the `gemfile_lock_deps`
    # repository rule.
    gemfile_locks = [
        "//gems/sorbet/test/snapshot:{}/src/Gemfile.lock".format(test)
        for test in [
            "partial/bad-hash",
            "partial/bad-t",
            "partial/codecov",
            "partial/create-config",
            "partial/explosive-object",
            "partial/fake-object",
            "partial/fake-rails",
            "partial/local_gem",
            "partial/non-utf-8-file",
            "partial/rails6",
            "partial/rails-double-require",
            "partial/real_singleton_class",
            "partial/rspec-lots",
            "partial/stack_master",
            "partial/stupidedi",
            "partial/typed-ignore",
            "partial/webmock",
            "total/empty",
            "total/sorbet-runtime",
        ]
    ],
)

BAZEL_VERSION = "3.4.1"

BAZEL_INSTALLER_VERSION_linux_SHA = "9808adad931ac652e8ff5022a74507c532250c2091d21d6aebc7064573669cc5"

BAZEL_INSTALLER_VERSION_darwin_SHA = "b168b9c4186916cd07922b1155bca14eecc812729669f1fdbab141f3f4eee2a0"

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "io_bazel_rules_rust",
    strip_prefix = "rules_rust-fdf9655ba95616e0314b4e0ebab40bb0c5fe005c",
    urls = [
        # Master branch as of 2019-10-07
        "https://github.com/bazelbuild/rules_rust/archive/fdf9655ba95616e0314b4e0ebab40bb0c5fe005c.tar.gz",
    ],
)

http_archive(
    name = "bazel_skylib",
    sha256 = "9a737999532daca978a158f94e77e9af6a6a169709c0cee274f0a4c3359519bd",
    strip_prefix = "bazel-skylib-1.0.0",
    url = "https://github.com/bazelbuild/bazel-skylib/archive/1.0.0.tar.gz",
)

load("@io_bazel_rules_rust//rust:repositories.bzl", "rust_repositories")

rust_repositories(
    edition = "2018",
    version = "1.46.0",
)

load("@io_bazel_rules_rust//:workspace.bzl", "bazel_version")

bazel_version(name = "bazel_version")
load("//third_party/cargo:crates.bzl", "raze_fetch_remote_crates")
raze_fetch_remote_crates()
