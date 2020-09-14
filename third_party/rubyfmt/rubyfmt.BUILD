load("@io_bazel_rules_rust//rust:rust.bzl", "rust_library")

rust_library(
    name = "ripper_deserialize",
    srcs = ["librubyfmt/ripper_deserialize/src/lib.rs"],
    crate_type = "proc-macro",
    edition = "2018",
    deps = [
        "@com_stripe_ruby_typer//third_party/cargo:proc_macro2",
        "@com_stripe_ruby_typer//third_party/cargo:quote",
        "@com_stripe_ruby_typer//third_party/cargo:syn",
    ],
)

rust_library(
    name = "librubyfmt",
    srcs = glob(["librubyfmt/src/**/*.rs"]),
    crate_type = "staticlib",
    data = [
        "librubyfmt/rubyfmt_lib.rb",
    ] + glob([
        "librubyfmt/ruby_checkout/ruby-2.6.6/ext/ripper/lib/**/*.rb",
    ]),
    edition = "2018",
    proc_macro_deps = [
        ":ripper_deserialize",
    ],
    deps = [
        "@com_stripe_ruby_typer//third_party/cargo:backtrace",
        "@com_stripe_ruby_typer//third_party/cargo:bytecount",
        "@com_stripe_ruby_typer//third_party/cargo:jemallocator",
        "@com_stripe_ruby_typer//third_party/cargo:libc",
        "@com_stripe_ruby_typer//third_party/cargo:log",
        "@com_stripe_ruby_typer//third_party/cargo:serde",
        "@com_stripe_ruby_typer//third_party/cargo:serde_json",
        "@com_stripe_ruby_typer//third_party/cargo:simplelog",
    ],
)
