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
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_binary",
    "rust_library",
    "rust_test",
)

# Unsupported target "average" with type "bench" omitted
# Unsupported target "average" with type "test" omitted
# Unsupported target "build-script-build" with type "custom-build" omitted
# Unsupported target "gcd" with type "bench" omitted

rust_library(
    name = "num_integer",
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
    version = "0.1.43",
    deps = [
        "@raze__num_traits__0_2_12//:num_traits",
    ],
)

# Unsupported target "roots" with type "bench" omitted
# Unsupported target "roots" with type "test" omitted
