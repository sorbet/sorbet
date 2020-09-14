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
    name = "serde",
    crate_type = "lib",
    deps = [
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    proc_macro_deps = [
        "@raze__serde_derive__1_0_113//:serde_derive",
    ],
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "1.0.113",
    tags = ["cargo-raze"],
    crate_features = [
        "default",
        "derive",
        "serde_derive",
        "std",
    ],
)

