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
  "notice", # Apache-2.0 from expression "Apache-2.0 OR MIT"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "addr2line" with type "example" omitted

rust_library(
    name = "addr2line",
    crate_type = "lib",
    deps = [
        "//third_party/cargo/vendor/gimli-0.22.0:gimli",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.13.0",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

# Unsupported target "correctness" with type "test" omitted
# Unsupported target "output_equivalence" with type "test" omitted
# Unsupported target "parse" with type "test" omitted
