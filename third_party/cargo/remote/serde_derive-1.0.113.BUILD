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

# Unsupported target "build-script-build" with type "custom-build" omitted

rust_library(
    name = "serde_derive",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        "default",
    ],
    crate_root = "src/lib.rs",
    crate_type = "proc-macro",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    tags = ["cargo-raze"],
    version = "1.0.113",
    deps = [
        "@raze__proc_macro2__1_0_10//:proc_macro2",
        "@raze__quote__1_0_3//:quote",
        "@raze__syn__1_0_17//:syn",
    ],
)
