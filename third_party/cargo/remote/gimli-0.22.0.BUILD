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

# Unsupported target "bench" with type "bench" omitted
# Unsupported target "convert_self" with type "test" omitted
# Unsupported target "dwarf-validate" with type "example" omitted
# Unsupported target "dwarfdump" with type "example" omitted

rust_library(
    name = "gimli",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        "read",
    ],
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    tags = ["cargo-raze"],
    version = "0.22.0",
    deps = [
    ],
)

# Unsupported target "parse_self" with type "test" omitted
# Unsupported target "simple" with type "example" omitted
# Unsupported target "simple_line" with type "example" omitted
