package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_rust//rust:rust.bzl", "rust_library")

rust_library(
    name = "ripper_deserialize",
    srcs = ["src/lib.rs"],
    crate_type = "proc-macro",
    deps = [
        "//third_party/cargo/vendor/proc-macro2-1.0.10:proc_macro2",
        "//third_party/cargo/vendor/quote-1.0.3:quote",
        "//third_party/cargo/vendor/syn-1.0.17:syn",
    ],
    edition = "2018",
)
