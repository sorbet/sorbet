"""
@generated
cargo-raze crate workspace functions

DO NOT EDIT! Replaced on runs of cargo-raze
"""
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

def _new_http_archive(name, **kwargs):
    if not native.existing_rule(name):
        http_archive(name=name, **kwargs)

def _new_git_repository(name, **kwargs):
    if not native.existing_rule(name):
        new_git_repository(name=name, **kwargs)

def raze_fetch_remote_crates():

    _new_http_archive(
        name = "raze__addr2line__0_13_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/addr2line/addr2line-0.13.0.crate",
        type = "tar.gz",
        strip_prefix = "addr2line-0.13.0",
        build_file = Label("//third_party/cargo/remote:addr2line-0.13.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__adler__0_2_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/adler/adler-0.2.3.crate",
        type = "tar.gz",
        strip_prefix = "adler-0.2.3",
        build_file = Label("//third_party/cargo/remote:adler-0.2.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__autocfg__1_0_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/autocfg/autocfg-1.0.1.crate",
        type = "tar.gz",
        strip_prefix = "autocfg-1.0.1",
        build_file = Label("//third_party/cargo/remote:autocfg-1.0.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__backtrace__0_3_50",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/backtrace/backtrace-0.3.50.crate",
        type = "tar.gz",
        strip_prefix = "backtrace-0.3.50",
        build_file = Label("//third_party/cargo/remote:backtrace-0.3.50.BUILD"),
    )

    _new_http_archive(
        name = "raze__bytecount__0_6_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/bytecount/bytecount-0.6.0.crate",
        type = "tar.gz",
        strip_prefix = "bytecount-0.6.0",
        build_file = Label("//third_party/cargo/remote:bytecount-0.6.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__cc__1_0_59",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/cc/cc-1.0.59.crate",
        type = "tar.gz",
        strip_prefix = "cc-1.0.59",
        build_file = Label("//third_party/cargo/remote:cc-1.0.59.BUILD"),
    )

    _new_http_archive(
        name = "raze__cfg_if__0_1_10",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/cfg-if/cfg-if-0.1.10.crate",
        type = "tar.gz",
        strip_prefix = "cfg-if-0.1.10",
        build_file = Label("//third_party/cargo/remote:cfg-if-0.1.10.BUILD"),
    )

    _new_http_archive(
        name = "raze__chrono__0_4_15",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/chrono/chrono-0.4.15.crate",
        type = "tar.gz",
        strip_prefix = "chrono-0.4.15",
        build_file = Label("//third_party/cargo/remote:chrono-0.4.15.BUILD"),
    )

    _new_http_archive(
        name = "raze__gimli__0_22_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/gimli/gimli-0.22.0.crate",
        type = "tar.gz",
        strip_prefix = "gimli-0.22.0",
        build_file = Label("//third_party/cargo/remote:gimli-0.22.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__itoa__0_4_6",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/itoa/itoa-0.4.6.crate",
        type = "tar.gz",
        strip_prefix = "itoa-0.4.6",
        build_file = Label("//third_party/cargo/remote:itoa-0.4.6.BUILD"),
    )

    _new_http_archive(
        name = "raze__libc__0_2_77",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/libc/libc-0.2.77.crate",
        type = "tar.gz",
        strip_prefix = "libc-0.2.77",
        build_file = Label("//third_party/cargo/remote:libc-0.2.77.BUILD"),
    )

    _new_http_archive(
        name = "raze__log__0_4_11",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/log/log-0.4.11.crate",
        type = "tar.gz",
        strip_prefix = "log-0.4.11",
        build_file = Label("//third_party/cargo/remote:log-0.4.11.BUILD"),
    )

    _new_http_archive(
        name = "raze__miniz_oxide__0_4_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/miniz_oxide/miniz_oxide-0.4.2.crate",
        type = "tar.gz",
        strip_prefix = "miniz_oxide-0.4.2",
        build_file = Label("//third_party/cargo/remote:miniz_oxide-0.4.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__num_integer__0_1_43",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/num-integer/num-integer-0.1.43.crate",
        type = "tar.gz",
        strip_prefix = "num-integer-0.1.43",
        build_file = Label("//third_party/cargo/remote:num-integer-0.1.43.BUILD"),
    )

    _new_http_archive(
        name = "raze__num_traits__0_2_12",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/num-traits/num-traits-0.2.12.crate",
        type = "tar.gz",
        strip_prefix = "num-traits-0.2.12",
        build_file = Label("//third_party/cargo/remote:num-traits-0.2.12.BUILD"),
    )

    _new_http_archive(
        name = "raze__object__0_20_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/object/object-0.20.0.crate",
        type = "tar.gz",
        strip_prefix = "object-0.20.0",
        build_file = Label("//third_party/cargo/remote:object-0.20.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__proc_macro2__1_0_10",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/proc-macro2/proc-macro2-1.0.10.crate",
        type = "tar.gz",
        strip_prefix = "proc-macro2-1.0.10",
        build_file = Label("//third_party/cargo/remote:proc-macro2-1.0.10.BUILD"),
    )

    _new_http_archive(
        name = "raze__quote__1_0_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/quote/quote-1.0.3.crate",
        type = "tar.gz",
        strip_prefix = "quote-1.0.3",
        build_file = Label("//third_party/cargo/remote:quote-1.0.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__rustc_demangle__0_1_16",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rustc-demangle/rustc-demangle-0.1.16.crate",
        type = "tar.gz",
        strip_prefix = "rustc-demangle-0.1.16",
        build_file = Label("//third_party/cargo/remote:rustc-demangle-0.1.16.BUILD"),
    )

    _new_http_archive(
        name = "raze__ryu__1_0_5",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/ryu/ryu-1.0.5.crate",
        type = "tar.gz",
        strip_prefix = "ryu-1.0.5",
        build_file = Label("//third_party/cargo/remote:ryu-1.0.5.BUILD"),
    )

    _new_http_archive(
        name = "raze__serde__1_0_113",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/serde/serde-1.0.113.crate",
        type = "tar.gz",
        strip_prefix = "serde-1.0.113",
        build_file = Label("//third_party/cargo/remote:serde-1.0.113.BUILD"),
    )

    _new_http_archive(
        name = "raze__serde_derive__1_0_113",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/serde_derive/serde_derive-1.0.113.crate",
        type = "tar.gz",
        strip_prefix = "serde_derive-1.0.113",
        build_file = Label("//third_party/cargo/remote:serde_derive-1.0.113.BUILD"),
    )

    _new_http_archive(
        name = "raze__serde_json__1_0_57",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/serde_json/serde_json-1.0.57.crate",
        type = "tar.gz",
        strip_prefix = "serde_json-1.0.57",
        build_file = Label("//third_party/cargo/remote:serde_json-1.0.57.BUILD"),
    )

    _new_http_archive(
        name = "raze__simplelog__0_8_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/simplelog/simplelog-0.8.0.crate",
        type = "tar.gz",
        strip_prefix = "simplelog-0.8.0",
        build_file = Label("//third_party/cargo/remote:simplelog-0.8.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__syn__1_0_17",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/syn/syn-1.0.17.crate",
        type = "tar.gz",
        strip_prefix = "syn-1.0.17",
        build_file = Label("//third_party/cargo/remote:syn-1.0.17.BUILD"),
    )

    _new_http_archive(
        name = "raze__termcolor__1_1_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/termcolor/termcolor-1.1.0.crate",
        type = "tar.gz",
        strip_prefix = "termcolor-1.1.0",
        build_file = Label("//third_party/cargo/remote:termcolor-1.1.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__time__0_1_44",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/time/time-0.1.44.crate",
        type = "tar.gz",
        strip_prefix = "time-0.1.44",
        build_file = Label("//third_party/cargo/remote:time-0.1.44.BUILD"),
    )

    _new_http_archive(
        name = "raze__unicode_xid__0_2_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/unicode-xid/unicode-xid-0.2.1.crate",
        type = "tar.gz",
        strip_prefix = "unicode-xid-0.2.1",
        build_file = Label("//third_party/cargo/remote:unicode-xid-0.2.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__wasi__0_10_0_wasi_snapshot_preview1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/wasi/wasi-0.10.0+wasi-snapshot-preview1.crate",
        type = "tar.gz",
        strip_prefix = "wasi-0.10.0+wasi-snapshot-preview1",
        build_file = Label("//third_party/cargo/remote:wasi-0.10.0+wasi-snapshot-preview1.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi__0_3_9",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi/winapi-0.3.9.crate",
        type = "tar.gz",
        strip_prefix = "winapi-0.3.9",
        build_file = Label("//third_party/cargo/remote:winapi-0.3.9.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_i686_pc_windows_gnu__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-i686-pc-windows-gnu/winapi-i686-pc-windows-gnu-0.4.0.crate",
        type = "tar.gz",
        strip_prefix = "winapi-i686-pc-windows-gnu-0.4.0",
        build_file = Label("//third_party/cargo/remote:winapi-i686-pc-windows-gnu-0.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_util__0_1_5",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-util/winapi-util-0.1.5.crate",
        type = "tar.gz",
        strip_prefix = "winapi-util-0.1.5",
        build_file = Label("//third_party/cargo/remote:winapi-util-0.1.5.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_x86_64_pc_windows_gnu__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-x86_64-pc-windows-gnu/winapi-x86_64-pc-windows-gnu-0.4.0.crate",
        type = "tar.gz",
        strip_prefix = "winapi-x86_64-pc-windows-gnu-0.4.0",
        build_file = Label("//third_party/cargo/remote:winapi-x86_64-pc-windows-gnu-0.4.0.BUILD"),
    )

