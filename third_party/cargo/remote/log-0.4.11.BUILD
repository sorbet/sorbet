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


# Unsupported target "build-script-build" with type "custom-build" omitted
# Unsupported target "filters" with type "test" omitted

rust_library(
    name = "log",
    crate_type = "lib",
    deps = [
        "@raze__cfg_if__0_1_10//:cfg_if",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
        "--cfg=atomic_cas",
    ],
    version = "0.4.11",
    tags = ["cargo-raze"],
    crate_features = [
        "max_level_debug",
        "release_max_level_warn",
        "std",
    ],
)

# Unsupported target "macros" with type "test" omitted
