load("@io_bazel_rules_rust//rust:rust.bzl", "rust_library")

rust_library(
    name = "ripper_deserialize",
    srcs = ["librubyfmt/ripper_deserialize/src/lib.rs"],
    crate_type = "proc-macro",
    edition = "2018",
    deps = [
        "@com_stripe_ruby_typer//third_party/cargo:proc_macro2",
        "@com_stripe_ruby_typer//third_party/cargo:quote",
        "@com_stripe_ruby_typer//third_party/cargo:syn",
    ],
)
