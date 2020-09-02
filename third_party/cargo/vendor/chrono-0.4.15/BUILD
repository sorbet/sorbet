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


# Unsupported target "chrono" with type "bench" omitted

rust_library(
    name = "chrono",
    crate_type = "lib",
    deps = [
        "//third_party/cargo/vendor/num-integer-0.1.43:num_integer",
        "//third_party/cargo/vendor/num-traits-0.2.12:num_traits",
        "//third_party/cargo/vendor/time-0.1.44:time",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.4.15",
    tags = ["cargo-raze"],
    crate_features = [
        "clock",
        "default",
        "std",
        "time",
    ],
)

# Unsupported target "serde" with type "bench" omitted
# Unsupported target "wasm" with type "test" omitted
