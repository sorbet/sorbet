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

    libffi_version = "3.4.5"
    http_archive(
        name = "libffi",
        urls = _github_public_urls("libffi/libffi/releases/download/v{}/libffi-{}.tar.gz".format(libffi_version, libffi_version)),
        sha256 = "96fff4e589e3b239d888d9aa44b3ff30693c2ba1617f953925a70ddebcc102b2",
        strip_prefix = "libffi-{}".format(libffi_version),
        build_file = "@com_stripe_ruby_typer//third_party/ruby:libffi.BUILD",
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
        urls = _rubygems_urls("rubygems-update-3.5.4.gem"),
        sha256 = "41d4c93a79426a7e034080cc367c696ee0ae5c26fcfef20bb58f950031c95924",
    )

    # Pre Ruby 3.0 needs an older rubygems
    http_file(
        name = "rubygems_update_stripe_ruby2",
        urls = _rubygems_urls("rubygems-update-3.3.3.gem"),
        sha256 = "610aef544e0c15ff3cd5492dff3f5f46bd2062896f4f62c7191432c6f1d681c9",
    )

    ruby_build = "@com_stripe_ruby_typer//third_party/ruby:ruby.BUILD"
    ruby_3_3_build = "@com_stripe_ruby_typer//third_party/ruby:ruby_3_3.BUILD"

    http_archive(
        name = "sorbet_ruby_3_1",
        urls = _ruby_urls("3.1/ruby-3.1.4.tar.gz"),
        sha256 = "a3d55879a0dfab1d7141fdf10d22a07dbf8e5cdc4415da1bde06127d5cc3c7b6",
        strip_prefix = "ruby-3.1.4",
        build_file = "@com_stripe_ruby_typer//third_party/ruby:ruby.BUILD",
        patches = [
            "@com_stripe_ruby_typer//third_party/ruby:gc-add-need-major-by-3_1.patch",  # https://github.com/ruby/ruby/pull/6791
            "@com_stripe_ruby_typer//third_party/ruby:thp.patch",
            "@com_stripe_ruby_typer//third_party/ruby:gc-t-none-context.patch",
            "@com_stripe_ruby_typer//third_party/ruby:gc-more-t-none-context.patch",
            "@com_stripe_ruby_typer//third_party/ruby:gc-write-barrier-cme.patch",
        ],
    )

    http_archive(
        name = "sorbet_ruby_3_2",
        urls = _ruby_urls("3.2/ruby-3.2.2.tar.gz"),
        sha256 = "96c57558871a6748de5bc9f274e93f4b5aad06cd8f37befa0e8d94e7b8a423bc",
        strip_prefix = "ruby-3.2.2",
        build_file = ruby_build,
    )

    http_archive(
        name = "sorbet_ruby_3_3_preview",
        urls = _ruby_urls("3.3/ruby-3.3.0-preview2.tar.gz"),
        sha256 = "30ce8b0fe11b37b5ac088f5a5765744b935eac45bb89a9e381731533144f5991",
        strip_prefix = "ruby-3.3.0-preview2",
        build_file = ruby_3_3_build,
    )

    http_archive(
        name = "sorbet_ruby_3_3",
        urls = _ruby_urls("3.3/ruby-3.3.0.tar.gz"),
        sha256 = "96518814d9832bece92a85415a819d4893b307db5921ae1f0f751a9a89a56b7d",
        strip_prefix = "ruby-3.3.0",
        build_file = ruby_3_3_build,
        patch_tool = "patch",
        patch_args = ["-p1"],
        patches = [
            "@com_stripe_ruby_typer//third_party/ruby:ldflags.patch",
            "@com_stripe_ruby_typer//third_party/ruby:fix_grapheme_clusters_3_3_only.patch",
            "@com_stripe_ruby_typer//third_party/ruby:10151_no_hash_allocate_static_kwargs_3_3_only.patch",
            "@com_stripe_ruby_typer//third_party/ruby:10306_no_hash_allocate_static_kwargs_3_3_only.patch",
            "@com_stripe_ruby_typer//third_party/ruby:9385-aarch64-fibers-crash-backport.patch",
        ],
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
