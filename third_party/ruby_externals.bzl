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

    # Pre Ruby 3.0 needs an older rubygems (3.2.x works better with Ruby 2.7's bundled RubyGems 3.1.4)
    http_file(
        name = "rubygems_update_stripe_ruby2",
        urls = _rubygems_urls("rubygems-update-3.2.33.gem"),
        sha256 = "46862bd39dd078789d1cc7e2359772e50b33880a28b3eb83f80d42eec7e5a7e2",
    )

    ruby_build = "@com_stripe_ruby_typer//third_party/ruby:ruby.BUILD"
    ruby_2_build = "@com_stripe_ruby_typer//third_party/ruby:ruby_2.BUILD"
    ruby_3_3_build = "@com_stripe_ruby_typer//third_party/ruby:ruby_3_3.BUILD"
    ruby_for_compiler_build = "@com_stripe_ruby_typer//third_party/ruby:ruby_for_compiler.BUILD"

    http_archive(
        name = "sorbet_ruby_2_6",
        urls = _ruby_urls("2.6/ruby-2.6.5.tar.gz"),
        sha256 = "66976b716ecc1fd34f9b7c3c2b07bbd37631815377a2e3e85a5b194cfdcbed7d",
        strip_prefix = "ruby-2.6.5",
        build_file = ruby_2_build,
    )

    urls = _ruby_urls("2.7/ruby-2.7.2.tar.gz")
    sha256 = "6e5706d0d4ee4e1e2f883db9d768586b4d06567debea353c796ec45e8321c3d4"
    strip_prefix = "ruby-2.7.2"

    http_archive(
        name = "sorbet_ruby_2_7_unpatched",
        urls = urls,
        sha256 = sha256,
        strip_prefix = strip_prefix,
        build_file = ruby_2_build,
    )

    http_archive(
        name = "sorbet_ruby_2_7",
        urls = urls,
        sha256 = sha256,
        strip_prefix = strip_prefix,
        build_file = ruby_2_build,
        patches = [
            "@com_stripe_ruby_typer//third_party/ruby:gc-remove-write-barrier.patch",
            "@com_stripe_ruby_typer//third_party/ruby:dtoa.patch",
            "@com_stripe_ruby_typer//third_party/ruby:penelope_procc.patch",
            "@com_stripe_ruby_typer//third_party/ruby:gc-fix-malloc-increase-calculation.patch",  # https://github.com/ruby/ruby/pull/4860
            "@com_stripe_ruby_typer//third_party/ruby:gc-add-need-major-by.patch",  # https://github.com/ruby/ruby/pull/6791
            "@com_stripe_ruby_typer//third_party/ruby:thp.patch",
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
            "@com_stripe_ruby_typer//third_party/ruby:thp.patch",
            "@com_stripe_ruby_typer//third_party/ruby:gc-t-none-context.patch",
            "@com_stripe_ruby_typer//third_party/ruby:gc-more-t-none-context.patch",
            "@com_stripe_ruby_typer//third_party/ruby:gc-write-barrier-cme.patch",
        ],
    )

    http_archive(
        name = "sorbet_ruby_3_3",
        urls = _ruby_urls("3.3/ruby-3.3.1.tar.gz"),
        sha256 = "8dc2af2802cc700cd182d5430726388ccf885b3f0a14fcd6a0f21ff249c9aa99",
        strip_prefix = "ruby-3.3.1",
        build_file = ruby_3_3_build,
        patch_tool = "patch",
        patch_args = ["-p1"],
        patches = [
            "@com_stripe_ruby_typer//third_party/ruby:ldflags.patch",
            "@com_stripe_ruby_typer//third_party/ruby:fix_grapheme_clusters_3_3_only.patch",
            "@com_stripe_ruby_typer//third_party/ruby:10151_no_hash_allocate_static_kwargs_3_3_only.patch",
            "@com_stripe_ruby_typer//third_party/ruby:10306_no_hash_allocate_static_kwargs_3_3_only.patch",
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
