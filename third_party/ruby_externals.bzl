load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

# We define our externals here instead of directly in WORKSPACE
def register_ruby_dependencies():
    libyaml_version = "0.2.5"
    http_archive(
        name = "libyaml",
        url = "https://github.com/yaml/libyaml/releases/download/{}/yaml-{}.tar.gz".format(libyaml_version, libyaml_version),
        sha256 = "c642ae9b75fee120b2d96c712538bd2cf283228d2337df2cf2988e3c02678ef4",
        strip_prefix = "yaml-{}".format(libyaml_version),
        build_file = "@com_stripe_ruby_typer//third_party/ruby:libyaml.BUILD",
    )

    libffi_version = "3.4.5"
    http_archive(
        name = "libffi",
        url = "https://github.com/libffi/libffi/releases/download/v{}/libffi-{}.tar.gz".format(libffi_version, libffi_version),
        sha256 = "96fff4e589e3b239d888d9aa44b3ff30693c2ba1617f953925a70ddebcc102b2",
        strip_prefix = "libffi-{}".format(libffi_version),
        build_file = "@com_stripe_ruby_typer//third_party/ruby:libffi.BUILD",
    )

    http_archive(
        name = "rules_rust",
        sha256 = "25209daff2ba21e818801c7b2dab0274c43808982d6aea9f796d899db6319146",
        url = "https://github.com/bazelbuild/rules_rust/releases/download/0.21.1/rules_rust-v0.21.1.tar.gz",
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
        url = "https://cache.ruby-lang.org/pub/ruby/3.1/ruby-3.1.4.tar.gz",
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
        name = "sorbet_ruby_3_3",
        url = "https://cache.ruby-lang.org/pub/ruby/3.3/ruby-3.3.3.tar.gz",
        sha256 = "83c05b2177ee9c335b631b29b8c077b4770166d02fa527f3a9f6a40d13f3cce2",
        strip_prefix = "ruby-3.3.3",
        build_file = ruby_3_3_build,
        patch_tool = "patch",
        patch_args = ["-p1"],
        patches = [
            "@com_stripe_ruby_typer//third_party/ruby:ldflags.patch",
            "@com_stripe_ruby_typer//third_party/ruby:10151_no_hash_allocate_static_kwargs_3_3_only.patch",
            "@com_stripe_ruby_typer//third_party/ruby:10306_no_hash_allocate_static_kwargs_3_3_only.patch",
            "@com_stripe_ruby_typer//third_party/ruby:reinit_native_sched_lock.patch",
            "@com_stripe_ruby_typer//third_party/ruby:10899-avoid-unnecessary-writes-in-gc-marking.patch",
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
