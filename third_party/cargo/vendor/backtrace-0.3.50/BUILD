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


# Unsupported target "accuracy" with type "test" omitted
# Unsupported target "backtrace" with type "example" omitted

rust_library(
    name = "backtrace",
    crate_type = "lib",
    deps = [
        "//third_party/cargo/vendor/addr2line-0.13.0:addr2line",
        "//third_party/cargo/vendor/cfg-if-0.1.10:cfg_if",
        "//third_party/cargo/vendor/libc-0.2.76:libc",
        "//third_party/cargo/vendor/miniz_oxide-0.4.1:miniz_oxide",
        "//third_party/cargo/vendor/object-0.20.0:object",
        "//third_party/cargo/vendor/rustc-demangle-0.1.16:rustc_demangle",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.3.50",
    tags = ["cargo-raze"],
    crate_features = [
        "addr2line",
        "default",
        "gimli-symbolize",
        "miniz_oxide",
        "object",
        "std",
    ],
)

# Unsupported target "benchmarks" with type "bench" omitted
# Unsupported target "concurrent-panics" with type "test" omitted
# Unsupported target "long_fn_name" with type "test" omitted
# Unsupported target "raw" with type "example" omitted
# Unsupported target "skip_inner_frames" with type "test" omitted
# Unsupported target "smoke" with type "test" omitted
