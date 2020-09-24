"""
@generated
cargo-raze crate workspace functions

DO NOT EDIT! Replaced on runs of cargo-raze
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

def _new_http_archive(name, **kwargs):
    if not native.existing_rule(name):
        http_archive(name = name, **kwargs)

def _new_git_repository(name, **kwargs):
    if not native.existing_rule(name):
        new_git_repository(name = name, **kwargs)

def raze_fetch_remote_crates():
    _new_http_archive(
        name = "raze__addr2line__0_13_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/addr2line/addr2line-0.13.0.crate",
        sha256 = "1b6a2d3371669ab3ca9797670853d61402b03d0b4b9ebf33d677dfa720203072",
        type = "tar.gz",
        strip_prefix = "addr2line-0.13.0",
        build_file = Label("//third_party/cargo/remote:addr2line-0.13.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__adler__0_2_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/adler/adler-0.2.3.crate",
        sha256 = "ee2a4ec343196209d6594e19543ae87a39f96d5534d7174822a3ad825dd6ed7e",
        type = "tar.gz",
        strip_prefix = "adler-0.2.3",
        build_file = Label("//third_party/cargo/remote:adler-0.2.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__autocfg__1_0_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/autocfg/autocfg-1.0.1.crate",
        sha256 = "cdb031dd78e28731d87d56cc8ffef4a8f36ca26c38fe2de700543e627f8a464a",
        type = "tar.gz",
        strip_prefix = "autocfg-1.0.1",
        build_file = Label("//third_party/cargo/remote:autocfg-1.0.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__backtrace__0_3_50",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/backtrace/backtrace-0.3.50.crate",
        sha256 = "46254cf2fdcdf1badb5934448c1bcbe046a56537b3987d96c51a7afc5d03f293",
        type = "tar.gz",
        strip_prefix = "backtrace-0.3.50",
        build_file = Label("//third_party/cargo/remote:backtrace-0.3.50.BUILD"),
    )

    _new_http_archive(
        name = "raze__bytecount__0_6_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/bytecount/bytecount-0.6.0.crate",
        sha256 = "b0017894339f586ccb943b01b9555de56770c11cda818e7e3d8bd93f4ed7f46e",
        type = "tar.gz",
        strip_prefix = "bytecount-0.6.0",
        build_file = Label("//third_party/cargo/remote:bytecount-0.6.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__cc__1_0_59",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/cc/cc-1.0.59.crate",
        sha256 = "66120af515773fb005778dc07c261bd201ec8ce50bd6e7144c927753fe013381",
        type = "tar.gz",
        strip_prefix = "cc-1.0.59",
        build_file = Label("//third_party/cargo/remote:cc-1.0.59.BUILD"),
    )

    _new_http_archive(
        name = "raze__cfg_if__0_1_10",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/cfg-if/cfg-if-0.1.10.crate",
        sha256 = "4785bdd1c96b2a846b2bd7cc02e86b6b3dbf14e7e53446c4f54c92a361040822",
        type = "tar.gz",
        strip_prefix = "cfg-if-0.1.10",
        build_file = Label("//third_party/cargo/remote:cfg-if-0.1.10.BUILD"),
    )

    _new_http_archive(
        name = "raze__chrono__0_4_15",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/chrono/chrono-0.4.15.crate",
        sha256 = "942f72db697d8767c22d46a598e01f2d3b475501ea43d0db4f16d90259182d0b",
        type = "tar.gz",
        strip_prefix = "chrono-0.4.15",
        build_file = Label("//third_party/cargo/remote:chrono-0.4.15.BUILD"),
    )

    _new_http_archive(
        name = "raze__gimli__0_22_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/gimli/gimli-0.22.0.crate",
        sha256 = "aaf91faf136cb47367fa430cd46e37a788775e7fa104f8b4bcb3861dc389b724",
        type = "tar.gz",
        strip_prefix = "gimli-0.22.0",
        build_file = Label("//third_party/cargo/remote:gimli-0.22.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__itoa__0_4_6",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/itoa/itoa-0.4.6.crate",
        sha256 = "dc6f3ad7b9d11a0c00842ff8de1b60ee58661048eb8049ed33c73594f359d7e6",
        type = "tar.gz",
        strip_prefix = "itoa-0.4.6",
        build_file = Label("//third_party/cargo/remote:itoa-0.4.6.BUILD"),
    )

    _new_http_archive(
        name = "raze__libc__0_2_77",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/libc/libc-0.2.77.crate",
        sha256 = "f2f96b10ec2560088a8e76961b00d47107b3a625fecb76dedb29ee7ccbf98235",
        type = "tar.gz",
        strip_prefix = "libc-0.2.77",
        build_file = Label("//third_party/cargo/remote:libc-0.2.77.BUILD"),
    )

    _new_http_archive(
        name = "raze__log__0_4_11",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/log/log-0.4.11.crate",
        sha256 = "4fabed175da42fed1fa0746b0ea71f412aa9d35e76e95e59b192c64b9dc2bf8b",
        type = "tar.gz",
        strip_prefix = "log-0.4.11",
        build_file = Label("//third_party/cargo/remote:log-0.4.11.BUILD"),
    )

    _new_http_archive(
        name = "raze__miniz_oxide__0_4_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/miniz_oxide/miniz_oxide-0.4.2.crate",
        sha256 = "c60c0dfe32c10b43a144bad8fc83538c52f58302c92300ea7ec7bf7b38d5a7b9",
        type = "tar.gz",
        strip_prefix = "miniz_oxide-0.4.2",
        build_file = Label("//third_party/cargo/remote:miniz_oxide-0.4.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__num_integer__0_1_43",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/num-integer/num-integer-0.1.43.crate",
        sha256 = "8d59457e662d541ba17869cf51cf177c0b5f0cbf476c66bdc90bf1edac4f875b",
        type = "tar.gz",
        strip_prefix = "num-integer-0.1.43",
        build_file = Label("//third_party/cargo/remote:num-integer-0.1.43.BUILD"),
    )

    _new_http_archive(
        name = "raze__num_traits__0_2_12",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/num-traits/num-traits-0.2.12.crate",
        sha256 = "ac267bcc07f48ee5f8935ab0d24f316fb722d7a1292e2913f0cc196b29ffd611",
        type = "tar.gz",
        strip_prefix = "num-traits-0.2.12",
        build_file = Label("//third_party/cargo/remote:num-traits-0.2.12.BUILD"),
    )

    _new_http_archive(
        name = "raze__object__0_20_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/object/object-0.20.0.crate",
        sha256 = "1ab52be62400ca80aa00285d25253d7f7c437b7375c4de678f5405d3afe82ca5",
        type = "tar.gz",
        strip_prefix = "object-0.20.0",
        build_file = Label("//third_party/cargo/remote:object-0.20.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__proc_macro2__1_0_10",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/proc-macro2/proc-macro2-1.0.10.crate",
        sha256 = "df246d292ff63439fea9bc8c0a270bed0e390d5ebd4db4ba15aba81111b5abe3",
        type = "tar.gz",
        strip_prefix = "proc-macro2-1.0.10",
        build_file = Label("//third_party/cargo/remote:proc-macro2-1.0.10.BUILD"),
    )

    _new_http_archive(
        name = "raze__quote__1_0_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/quote/quote-1.0.3.crate",
        sha256 = "2bdc6c187c65bca4260c9011c9e3132efe4909da44726bad24cf7572ae338d7f",
        type = "tar.gz",
        strip_prefix = "quote-1.0.3",
        build_file = Label("//third_party/cargo/remote:quote-1.0.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__rustc_demangle__0_1_16",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rustc-demangle/rustc-demangle-0.1.16.crate",
        sha256 = "4c691c0e608126e00913e33f0ccf3727d5fc84573623b8d65b2df340b5201783",
        type = "tar.gz",
        strip_prefix = "rustc-demangle-0.1.16",
        build_file = Label("//third_party/cargo/remote:rustc-demangle-0.1.16.BUILD"),
    )

    _new_http_archive(
        name = "raze__ryu__1_0_5",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/ryu/ryu-1.0.5.crate",
        sha256 = "71d301d4193d031abdd79ff7e3dd721168a9572ef3fe51a1517aba235bd8f86e",
        type = "tar.gz",
        strip_prefix = "ryu-1.0.5",
        build_file = Label("//third_party/cargo/remote:ryu-1.0.5.BUILD"),
    )

    _new_http_archive(
        name = "raze__serde__1_0_113",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/serde/serde-1.0.113.crate",
        sha256 = "6135c78461981c79497158ef777264c51d9d0f4f3fc3a4d22b915900e42dac6a",
        type = "tar.gz",
        strip_prefix = "serde-1.0.113",
        build_file = Label("//third_party/cargo/remote:serde-1.0.113.BUILD"),
    )

    _new_http_archive(
        name = "raze__serde_derive__1_0_113",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/serde_derive/serde_derive-1.0.113.crate",
        sha256 = "93c5eaa17d0954cb481cdcfffe9d84fcfa7a1a9f2349271e678677be4c26ae31",
        type = "tar.gz",
        strip_prefix = "serde_derive-1.0.113",
        build_file = Label("//third_party/cargo/remote:serde_derive-1.0.113.BUILD"),
    )

    _new_http_archive(
        name = "raze__serde_json__1_0_57",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/serde_json/serde_json-1.0.57.crate",
        sha256 = "164eacbdb13512ec2745fb09d51fd5b22b0d65ed294a1dcf7285a360c80a675c",
        type = "tar.gz",
        strip_prefix = "serde_json-1.0.57",
        build_file = Label("//third_party/cargo/remote:serde_json-1.0.57.BUILD"),
    )

    _new_http_archive(
        name = "raze__simplelog__0_8_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/simplelog/simplelog-0.8.0.crate",
        sha256 = "2b2736f58087298a448859961d3f4a0850b832e72619d75adc69da7993c2cd3c",
        type = "tar.gz",
        strip_prefix = "simplelog-0.8.0",
        build_file = Label("//third_party/cargo/remote:simplelog-0.8.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__syn__1_0_17",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/syn/syn-1.0.17.crate",
        sha256 = "0df0eb663f387145cab623dea85b09c2c5b4b0aef44e945d928e682fce71bb03",
        type = "tar.gz",
        strip_prefix = "syn-1.0.17",
        build_file = Label("//third_party/cargo/remote:syn-1.0.17.BUILD"),
    )

    _new_http_archive(
        name = "raze__termcolor__1_1_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/termcolor/termcolor-1.1.0.crate",
        sha256 = "bb6bfa289a4d7c5766392812c0a1f4c1ba45afa1ad47803c11e1f407d846d75f",
        type = "tar.gz",
        strip_prefix = "termcolor-1.1.0",
        build_file = Label("//third_party/cargo/remote:termcolor-1.1.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__time__0_1_44",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/time/time-0.1.44.crate",
        sha256 = "6db9e6914ab8b1ae1c260a4ae7a49b6c5611b40328a735b21862567685e73255",
        type = "tar.gz",
        strip_prefix = "time-0.1.44",
        build_file = Label("//third_party/cargo/remote:time-0.1.44.BUILD"),
    )

    _new_http_archive(
        name = "raze__unicode_xid__0_2_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/unicode-xid/unicode-xid-0.2.1.crate",
        sha256 = "f7fe0bb3479651439c9112f72b6c505038574c9fbb575ed1bf3b797fa39dd564",
        type = "tar.gz",
        strip_prefix = "unicode-xid-0.2.1",
        build_file = Label("//third_party/cargo/remote:unicode-xid-0.2.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__wasi__0_10_0_wasi_snapshot_preview1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/wasi/wasi-0.10.0+wasi-snapshot-preview1.crate",
        sha256 = "1a143597ca7c7793eff794def352d41792a93c481eb1042423ff7ff72ba2c31f",
        type = "tar.gz",
        strip_prefix = "wasi-0.10.0+wasi-snapshot-preview1",
        build_file = Label("//third_party/cargo/remote:wasi-0.10.0+wasi-snapshot-preview1.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi__0_3_9",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi/winapi-0.3.9.crate",
        sha256 = "5c839a674fcd7a98952e593242ea400abe93992746761e38641405d28b00f419",
        type = "tar.gz",
        strip_prefix = "winapi-0.3.9",
        build_file = Label("//third_party/cargo/remote:winapi-0.3.9.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_i686_pc_windows_gnu__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-i686-pc-windows-gnu/winapi-i686-pc-windows-gnu-0.4.0.crate",
        sha256 = "ac3b87c63620426dd9b991e5ce0329eff545bccbbb34f3be09ff6fb6ab51b7b6",
        type = "tar.gz",
        strip_prefix = "winapi-i686-pc-windows-gnu-0.4.0",
        build_file = Label("//third_party/cargo/remote:winapi-i686-pc-windows-gnu-0.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_util__0_1_5",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-util/winapi-util-0.1.5.crate",
        sha256 = "70ec6ce85bb158151cae5e5c87f95a8e97d2c0c4b001223f33a334e3ce5de178",
        type = "tar.gz",
        strip_prefix = "winapi-util-0.1.5",
        build_file = Label("//third_party/cargo/remote:winapi-util-0.1.5.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_x86_64_pc_windows_gnu__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-x86_64-pc-windows-gnu/winapi-x86_64-pc-windows-gnu-0.4.0.crate",
        sha256 = "712e227841d057c1ee1cd2fb22fa7e5a5461ae8e48fa2ca79ec42cfc1931183f",
        type = "tar.gz",
        strip_prefix = "winapi-x86_64-pc-windows-gnu-0.4.0",
        build_file = Label("//third_party/cargo/remote:winapi-x86_64-pc-windows-gnu-0.4.0.BUILD"),
    )
