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



rust_library(
    name = "cc",
    crate_type = "lib",
    deps = [
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "1.0.59",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

# Unsupported target "cc_env" with type "test" omitted
# Unsupported target "cflags" with type "test" omitted
# Unsupported target "cxxflags" with type "test" omitted
rust_binary(
    # Prefix bin name to disambiguate from (probable) collision with lib name
    # N.B.: The exact form of this is subject to change.
    name = "cargo_bin_gcc_shim",
    deps = [
        # Binaries get an implicit dependency on their crate's lib
        ":cc",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/bin/gcc-shim.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "1.0.59",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

# Unsupported target "test" with type "test" omitted
