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
    "notice",  # Apache-2.0 from expression "Apache-2.0 OR MIT"
])

load(
    "@rules_rust//rust:rust.bzl",
    "rust_binary",
    "rust_library",
    "rust_test",
)

# Unsupported target "addr2line" with type "example" omitted

rust_library(
    name = "addr2line",
    srcs = glob(["**/*.rs"]),
    crate_features = [
    ],
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    tags = ["cargo-raze"],
    version = "0.13.0",
    deps = [
        "@raze__gimli__0_22_0//:gimli",
    ],
)

# Unsupported target "correctness" with type "test" omitted
# Unsupported target "output_equivalence" with type "test" omitted
# Unsupported target "parse" with type "test" omitted
