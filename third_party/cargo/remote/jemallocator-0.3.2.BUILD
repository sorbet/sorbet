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

# Unsupported target "background_thread_defaults" with type "test" omitted
# Unsupported target "background_thread_enabled" with type "test" omitted
# Unsupported target "ffi" with type "test" omitted
# Unsupported target "grow_in_place" with type "test" omitted

rust_library(
    name = "jemallocator",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        "background_threads_runtime_support",
        "default",
        "disable_initial_exec_tls",
    ],
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    tags = ["cargo-raze"],
    version = "0.3.2",
    deps = [
        "@raze__jemalloc_sys__0_3_2//:jemalloc_sys",
        "@raze__libc__0_2_77//:libc",
    ],
)

# Unsupported target "malloctl" with type "test" omitted
# Unsupported target "roundtrip" with type "bench" omitted
# Unsupported target "shrink_in_place" with type "test" omitted
# Unsupported target "smoke" with type "test" omitted
# Unsupported target "smoke_ffi" with type "test" omitted
# Unsupported target "usable_size" with type "test" omitted
