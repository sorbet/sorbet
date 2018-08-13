workspace(name = "com_stripe_ruby_typer")

load(":third_party/externals.bzl", "externals")

externals()

load("@io_bazel_rules_go//go:def.bzl", "go_rules_dependencies", "go_register_toolchains")

go_rules_dependencies()

go_register_toolchains()

BAZEL_VERSION = "0.16.0"

BAZEL_VERSION_SHA = "c730593916ef0ba62f3d113cc3a268e45f7e8039daf7b767c8641b6999bd49b1"
