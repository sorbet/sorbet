"""
@generated
cargo-raze crate build file.

DO NOT EDIT! Replaced on runs of cargo-raze
"""
package(default_visibility = [
  # Public for visibility by "@raze__crate__version//" targets.
  #
  # Prefer access through "//third_party/cargo", which limits external
  # visibility to explicit Cargo.toml dependencies.
  "//visibility:public",
])

licenses([
  "notice", # MIT from expression "MIT OR Apache-2.0"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "build-script-build" with type "custom-build" omitted

rust_library(
    name = "serde_derive",
    crate_type = "proc-macro",
    deps = [
        "//third_party/cargo/vendor/proc-macro2-1.0.10:proc_macro2",
        "//third_party/cargo/vendor/quote-1.0.3:quote",
        "//third_party/cargo/vendor/syn-1.0.17:syn",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "1.0.113",
    tags = ["cargo-raze"],
    crate_features = [
        "default",
    ],
)

