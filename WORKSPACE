workspace(name = "com_stripe_ruby_typer")

load("//third_party:externals.bzl", "register_sorbet_dependencies")

register_sorbet_dependencies()

load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain_10_0_0",
    absolute_paths = True,
    llvm_mirror_prefixes = [
        "https://sorbet-deps.s3-us-west-2.amazonaws.com/",
        "https://artifactory-content.stripe.build/artifactory/github-archives/llvm/llvm-project/releases/download/llvmorg-",
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-",
    ],
    llvm_version = "10.0.0",
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

load("@io_bazel_rules_rust//rust:repositories.bzl", "rust_repositories")

rust_repositories(
    edition = "2018",
    version = "1.46.0",
)

load("@io_bazel_rules_rust//:workspace.bzl", "bazel_version")

bazel_version(name = "bazel_version")

BAZEL_VERSION = "3.7.2"

BAZEL_INSTALLER_VERSION_linux_SHA = "8416ff3900075ed588869a5b6dcc97844f56834e5a8344a2e27ec34a1eaf847e"

BAZEL_INSTALLER_VERSION_darwin_SHA = "add8392086e3bce99cd01deaf08199c25de0e8d9ef823207e6be47e35750f3ba"
