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
    "notice",  # Apache-2.0 from expression "Apache-2.0 OR BSL-1.0"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_binary",
    "rust_library",
    "rust_test",
)

# Unsupported target "bench" with type "bench" omitted
# Unsupported target "build-script-build" with type "custom-build" omitted
# Unsupported target "common_test" with type "test" omitted
# Unsupported target "d2s_table_test" with type "test" omitted
# Unsupported target "d2s_test" with type "test" omitted
# Unsupported target "exhaustive" with type "test" omitted
# Unsupported target "f2s_test" with type "test" omitted

rust_library(
    name = "ryu",
    srcs = glob(["**/*.rs"]),
    crate_features = [
    ],
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    tags = ["cargo-raze"],
    version = "1.0.5",
    deps = [
    ],
)

# Unsupported target "s2d_test" with type "test" omitted
# Unsupported target "s2f_test" with type "test" omitted
# Unsupported target "upstream_benchmark" with type "example" omitted
