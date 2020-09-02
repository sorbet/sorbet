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
    name = "serde_json",
    crate_type = "lib",
    deps = [
        "//third_party/cargo/vendor/itoa-0.4.6:itoa",
        "//third_party/cargo/vendor/ryu-1.0.5:ryu",
        "//third_party/cargo/vendor/serde-1.0.113:serde",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "1.0.57",
    tags = ["cargo-raze"],
    crate_features = [
        "default",
        "std",
    ],
)

