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
    "notice",  # MIT from expression "MIT OR Apache-2.0"
])

load(
    "@rules_rust//rust:rust.bzl",
    "rust_binary",
    "rust_library",
    "rust_test",
)

rust_library(
    name = "simplelog",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        "default",
        "termcolor",
    ],
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    tags = ["cargo-raze"],
    version = "0.8.0",
    deps = [
        "@raze__chrono__0_4_15//:chrono",
        "@raze__log__0_4_11//:log",
        "@raze__termcolor__1_1_0//:termcolor",
    ],
)

# Unsupported target "usage" with type "example" omitted
