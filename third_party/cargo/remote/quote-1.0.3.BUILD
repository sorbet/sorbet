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

# Unsupported target "compiletest" with type "test" omitted

rust_library(
    name = "quote",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        "default",
        "proc-macro",
    ],
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    tags = ["cargo-raze"],
    version = "1.0.3",
    deps = [
        "@raze__proc_macro2__1_0_10//:proc_macro2",
    ],
)

# Unsupported target "test" with type "test" omitted
