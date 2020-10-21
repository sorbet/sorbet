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
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_binary",
    "rust_library",
    "rust_test",
)

# Unsupported target "integration" with type "test" omitted
# Unsupported target "nm" with type "example" omitted
# Unsupported target "objcopy" with type "example" omitted
# Unsupported target "objdump" with type "example" omitted

rust_library(
    name = "object",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        "coff",
        "elf",
        "macho",
        "pe",
        "read_core",
        "unaligned",
    ],
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    tags = ["cargo-raze"],
    version = "0.20.0",
    deps = [
    ],
)

# Unsupported target "parse_self" with type "test" omitted
