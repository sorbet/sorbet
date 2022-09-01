load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

# We define our externals here instead of directly in WORKSPACE
def register_ruby_dependencies():
    http_file(
        name = "bundler_stripe",
        urls = _rubygems_urls("bundler-1.17.3.gem"),
        sha256 = "bc4bf75b548b27451aa9f443b18c46a739dd22ad79f7a5f90b485376a67dc352",
    )

    http_file(
        name = "rubygems_update_stripe",
        urls = _rubygems_urls("rubygems-update-3.1.2.gem"),
        sha256 = "7bfe4e5e274191e56da8d127c79df10d9120feb8650e4bad29238f4b2773a661",
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
        name = "sorbet_ruby_3_0",
        urls = _ruby_urls("3.0/ruby-3.0.4.tar.gz"),
        sha256 = "70b47c207af04bce9acea262308fb42893d3e244f39a4abc586920a1c723722b",
        strip_prefix = "ruby-3.0.4",
        build_file = "@com_stripe_ruby_typer//third_party/ruby:ruby.BUILD",
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
