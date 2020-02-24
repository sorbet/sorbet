load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")
load("//third_party:sorbet_version.bzl", "SORBET_SHA256", "SORBET_VERSION")

# We define our externals here instead of directly in WORKSPACE
def sorbet_llvm_externals():
    use_local = False
    if not use_local:
        http_archive(
            name = "com_stripe_ruby_typer",
            url = "https://github.com/sorbet/sorbet/archive/{}.zip".format(SORBET_VERSION),
            sha256 = SORBET_SHA256,
            strip_prefix = "sorbet-{}".format(SORBET_VERSION),
        )
    else:
        native.local_repository(
            name = "com_stripe_ruby_typer",
            path = "../sorbet/",
        )

    http_archive(
        name = "llvm",
        url = "https://github.com/llvm/llvm-project/archive/c1a0a213378a458fbea1a5c77b315c7dce08fd05.tar.gz",  # llvm 9.0.1
        build_file = "@com_stripe_sorbet_llvm//third_party/llvm:llvm.autogenerated.BUILD",
        sha256 = "81a1a2ef99a780759b03dbcc926f11ce75acbdf227c1c66cce6f2057b58a962b",
        strip_prefix = "llvm-project-c1a0a213378a458fbea1a5c77b315c7dce08fd05/llvm",
    )

    http_archive(
        name = "zlib_archive",
        url = "https://zlib.net/zlib-1.2.11.tar.gz",
        build_file = "@com_stripe_sorbet_llvm//third_party:zlib.BUILD",
        sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
        strip_prefix = "zlib-1.2.11",
    )

    http_file(
        name = "bundler_stripe",
        urls = [
            "https://rubygems.org/downloads/bundler-1.17.3.gem",
            "https://intgems.local.corp.stripe.com:446/gems/bundler-1.17.3.gem",
        ],
        sha256 = "bc4bf75b548b27451aa9f443b18c46a739dd22ad79f7a5f90b485376a67dc352",
    )

    for apply_patch in [True, False]:
        url = "https://cache.ruby-lang.org/pub/ruby/2.6/ruby-2.6.5.tar.gz"
        sha256 = "66976b716ecc1fd34f9b7c3c2b07bbd37631815377a2e3e85a5b194cfdcbed7d"
        strip_prefix = "ruby-2.6.5"
        build_file = "@com_stripe_sorbet_llvm//third_party/ruby:ruby.BUILD"

        if apply_patch:
            http_archive(
                name = "sorbet_ruby",
                url = url,
                sha256 = sha256,
                strip_prefix = strip_prefix,
                build_file = build_file,
                patches = ["@com_stripe_sorbet_llvm//third_party/ruby:export-intrinsics.patch"],
                patch_tool = "patch",
            )
        else:
            http_archive(
                name = "sorbet_ruby_unpatched",
                url = url,
                sha256 = sha256,
                strip_prefix = strip_prefix,
                build_file = build_file,
            )
