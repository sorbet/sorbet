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

# Unsupported target "build-script-build" with type "custom-build" omitted
# Unsupported target "features" with type "test" omitted
# Unsupported target "marker" with type "test" omitted

rust_library(
    name = "proc_macro2",
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
        "--cfg=use_proc_macro",
    ],
    tags = ["cargo-raze"],
    version = "1.0.10",
    deps = [
        "@raze__unicode_xid__0_2_1//:unicode_xid",
    ],
)

# Unsupported target "test" with type "test" omitted
