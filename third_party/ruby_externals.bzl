load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

# We define our externals here instead of directly in WORKSPACE
def register_ruby_dependencies():
    libyaml_version = "0.2.5"
    http_archive(
        name = "libyaml",
        urls = _github_public_urls("yaml/libyaml/releases/download/{}/yaml-{}.tar.gz".format(libyaml_version, libyaml_version)),
        sha256 = "c642ae9b75fee120b2d96c712538bd2cf283228d2337df2cf2988e3c02678ef4",
        strip_prefix = "yaml-{}".format(libyaml_version),
        build_file = "@com_stripe_ruby_typer//third_party/ruby:libyaml.BUILD",
    )

    http_archive(
        name = "rules_rust",
        sha256 = "25209daff2ba21e818801c7b2dab0274c43808982d6aea9f796d899db6319146",
        urls = _github_public_urls("bazelbuild/rules_rust/releases/download/0.21.1/rules_rust-v0.21.1.tar.gz"),
    )

    http_file(
        name = "bundler_stripe",
        urls = _rubygems_urls("bundler-1.17.3.gem"),
        sha256 = "bc4bf75b548b27451aa9f443b18c46a739dd22ad79f7a5f90b485376a67dc352",
    )

    http_file(
        name = "rubygems_update_stripe",
        urls = _rubygems_urls("rubygems-update-3.3.3.gem"),
        sha256 = "610aef544e0c15ff3cd5492dff3f5f46bd2062896f4f62c7191432c6f1d681c9",
    )

    ruby_build = "@com_stripe_ruby_typer//third_party/ruby:ruby.BUILD"
    ruby_for_compiler_build = "@com_stripe_ruby_typer//third_party/ruby:ruby_for_compiler.BUILD"

    http_archive(
        name = "sorbet_ruby_2_6",
        urls = _ruby_urls("2.6/ruby-2.6.5.tar.gz"),
        sha256 = "66976b716ecc1fd34f9b7c3c2b07bbd37631815377a2e3e85a5b194cfdcbed7d",
        strip_prefix = "ruby-2.6.5",
        build_file = ruby_build,
    )

    urls = _ruby_urls("2.7/ruby-2.7.2.tar.gz")
    sha256 = "6e5706d0d4ee4e1e2f883db9d768586b4d06567debea353c796ec45e8321c3d4"
    strip_prefix = "ruby-2.7.2"

    http_archive(
        name = "sorbet_ruby_2_7_unpatched",
        urls = urls,
        sha256 = sha256,
        strip_prefix = strip_prefix,
        build_file = ruby_build,
    )

    http_archive(
        name = "sorbet_ruby_2_7",
        urls = urls,
        sha256 = sha256,
        strip_prefix = strip_prefix,
        build_file = ruby_build,
        patches = [
            "@com_stripe_ruby_typer//third_party/ruby:gc-remove-write-barrier.patch",
            "@com_stripe_ruby_typer//third_party/ruby:dtoa.patch",
            "@com_stripe_ruby_typer//third_party/ruby:penelope_procc.patch",
            "@com_stripe_ruby_typer//third_party/ruby:gc-fix-malloc-increase-calculation.patch",  # https://github.com/ruby/ruby/pull/4860
            "@com_stripe_ruby_typer//third_party/ruby:gc-add-need-major-by.patch",  # https://github.com/ruby/ruby/pull/6791
        ],
    )

    http_archive(
        name = "sorbet_ruby_2_7_for_compiler",
        urls = urls,
        sha256 = sha256,
        strip_prefix = strip_prefix,
        build_file = ruby_for_compiler_build,
        patches = [
            "@com_stripe_ruby_typer//third_party/ruby:sorbet_ruby_2_7_for_compiler.patch",
            "@com_stripe_ruby_typer//third_party/ruby:dtoa-p1.patch",
        ],
        patch_tool = "patch",
        patch_args = ["-p1"],
    )

    http_archive(
        name = "sorbet_ruby_3_1",
        urls = _ruby_urls("3.1/ruby-3.1.4.tar.gz"),
        sha256 = "a3d55879a0dfab1d7141fdf10d22a07dbf8e5cdc4415da1bde06127d5cc3c7b6",
        strip_prefix = "ruby-3.1.4",
        build_file = "@com_stripe_ruby_typer//third_party/ruby:ruby.BUILD",
        patches = [
            "@com_stripe_ruby_typer//third_party/ruby:gc-add-need-major-by-3_1.patch",  # https://github.com/ruby/ruby/pull/6791
        ],
    )

    http_archive(
        name = "sorbet_ruby_3_2",
        urls = _ruby_urls("3.2/ruby-3.2.2.tar.gz"),
        sha256 = "96c57558871a6748de5bc9f274e93f4b5aad06cd8f37befa0e8d94e7b8a423bc",
        strip_prefix = "ruby-3.2.2",
        build_file = ruby_build,
    )

def _rubygems_urls(gem):
    """
    Produce a url list that works both with rubygems, and stripe's internal gem cache.
    """
    return [
        "https://rubygems.org/downloads/{}".format(gem),
        "https://artifactory-content.stripe.build/artifactory/gems/gems/{}".format(gem),
    ]

def _ruby_urls(path):
    """
    Produce a url list that works both with ruby-lang.org, and stripe's internal artifact cache.
    """
    return [
        "https://cache.ruby-lang.org/pub/ruby/{}".format(path),
        "https://artifactory-content.stripe.build/artifactory/ruby-lang-cache/pub/ruby/{}".format(path),
    ]

def _github_public_urls(path):
    """
    Produce a url list that works both with github, and stripe's internal artifact cache.
    """
    return [
        "https://github.com/{}".format(path),
        "https://artifactory-content.stripe.build/artifactory/github-archives/{}".format(path),
    ]
