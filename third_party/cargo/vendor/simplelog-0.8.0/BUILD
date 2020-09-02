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



rust_library(
    name = "simplelog",
    crate_type = "lib",
    deps = [
        "//third_party/cargo/vendor/chrono-0.4.15:chrono",
        "//third_party/cargo/vendor/log-0.4.11:log",
        "//third_party/cargo/vendor/termcolor-1.1.0:termcolor",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.8.0",
    tags = ["cargo-raze"],
    crate_features = [
        "default",
        "termcolor",
    ],
)

# Unsupported target "usage" with type "example" omitted
