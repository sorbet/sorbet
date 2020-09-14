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

# Unsupported target "accuracy" with type "test" omitted
# Unsupported target "backtrace" with type "example" omitted

rust_library(
    name = "backtrace",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        "addr2line",
        "default",
        "gimli-symbolize",
        "miniz_oxide",
        "object",
        "std",
    ],
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    tags = ["cargo-raze"],
    version = "0.3.50",
    deps = [
        "@raze__addr2line__0_13_0//:addr2line",
        "@raze__cfg_if__0_1_10//:cfg_if",
        "@raze__libc__0_2_77//:libc",
        "@raze__miniz_oxide__0_4_2//:miniz_oxide",
        "@raze__object__0_20_0//:object",
        "@raze__rustc_demangle__0_1_16//:rustc_demangle",
    ],
)

# Unsupported target "benchmarks" with type "bench" omitted
# Unsupported target "concurrent-panics" with type "test" omitted
# Unsupported target "long_fn_name" with type "test" omitted
# Unsupported target "raw" with type "example" omitted
# Unsupported target "skip_inner_frames" with type "test" omitted
# Unsupported target "smoke" with type "test" omitted
