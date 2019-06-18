workspace(name = "com_stripe_ruby_typer")

load(":third_party/externals.bzl", "externals")

externals()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain",
    absolute_paths = True,
    llvm_version = "8.0.0",
)

load("@io_bazel_rules_go//go:def.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains()

load("@io_bazel_rules_ragel//ragel:ragel.bzl", "ragel_register_toolchains")

ragel_register_toolchains()

load("@io_bazel_rules_m4//m4:m4.bzl", "m4_register_toolchains")

m4_register_toolchains()

load("@io_bazel_rules_bison//bison:bison.bzl", "bison_register_toolchains")

bison_register_toolchains()

load("//third_party/gems:rules.bzl", "gemfile_lock_deps")

gemfile_lock_deps(
    name = "gems",
    gemfile_locks = [
        "//gems/sorbet/test/snapshot:{}/src/Gemfile.lock".format(test) for test in [
            "partial/bad-hash",
            "partial/bad-t",
            "partial/codecov",
            "partial/create-config",
            "partial/explosive-object",
            "partial/fake-object",
            "partial/fake-rails",
            # "partial/local_gem",
            "partial/non-utf-8-file",
            "partial/partially-sigged",
            "partial/rails-double-require",
            "partial/real_singleton_class",
            "partial/require_relative",
            "partial/rspec-lots",
            "partial/stack_master",
            "partial/stupidedi",
            "partial/typed-ignore",
            "partial/webmock",
            "total/empty",
        ]
    ],
)

BAZEL_INSTALLER_VERSION_linux_SHA = "328d5fa87a61e1f6e674a8f88c5ae54b8987eaf5a6c944257600c5029c8feef8"

BAZEL_INSTALLER_VERSION_darwin_SHA = "5e40dcf12a18990ffe5830fb5c83297aed090fd6e6c7c5b2eb720c19a33044fc"
