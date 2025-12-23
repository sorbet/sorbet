load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# We define our externals here instead of directly in WORKSPACE
# Many deps now come from BCR via MODULE.bazel
# These stay in WORKSPACE for toolchain compatibility: bazel_features, rules_cc
def register_sorbet_dependencies():
    http_archive(
        name = "bazel_features",
        sha256 = "a660027f5a87f13224ab54b8dc6e191693c554f2692fcca46e8e29ee7dabc43b",
        strip_prefix = "bazel_features-1.30.0",
        url = "https://github.com/bazel-contrib/bazel_features/releases/download/v1.30.0/bazel_features-v1.30.0.tar.gz",
    )

    http_archive(
        name = "prism",
        url = "https://github.com/ruby/prism/releases/download/v1.6.0/libprism-src.tar.gz",
        sha256 = "a643517b910c510c998a518e45a66f7f3460e5b32f80d64aa0021fed7c967d0f",
        build_file = "@com_stripe_ruby_typer//third_party:prism.BUILD",
        strip_prefix = "libprism-src",
    )

    # dtl now comes from MODULE.bazel with archive_override

    # yaml_cpp now comes from BCR via MODULE.bazel

    # spdlog now comes from BCR via MODULE.bazel

    # libprotobuf-mutator now comes from BCR via MODULE.bazel

    # lmdb now comes from MODULE.bazel with archive_override

    # rapidjson now comes from MODULE.bazel with archive_override

    # lz4 now comes from BCR via MODULE.bazel

    # pdqsort now comes from MODULE.bazel with archive_override

    http_archive(
        name = "jemalloc",
        url = "https://github.com/jemalloc/jemalloc/archive/20f9802e4f25922884448d9581c66d76cc905c0c.zip",  # 5.3
        sha256 = "1cc1ec93701868691c73b371eb87e5452257996279a42303a91caad355374439",
        build_file = "@com_stripe_ruby_typer//third_party:jemalloc.BUILD",
        strip_prefix = "jemalloc-20f9802e4f25922884448d9581c66d76cc905c0c",
    )

    # mimalloc now comes from BCR via MODULE.bazel

    # concurrentqueue now comes from MODULE.bazel with archive_override

    # statsd now comes from MODULE.bazel with archive_override

    # cxxopts now comes from MODULE.bazel with archive_override

    # rang now comes from MODULE.bazel with archive_override

    # xxhash now comes from BCR via MODULE.bazel

    http_archive(
        name = "com_grail_bazel_compdb",
        url = "https://github.com/grailbio/bazel-compilation-database/archive/6b9329e37295eab431f82af5fe24219865403e0f.zip",
        sha256 = "6cf0dc4b40023a26787cd7cdb629dccd26e2208c8a2f19e1dde4ca10c109c86c",
        strip_prefix = "bazel-compilation-database-6b9329e37295eab431f82af5fe24219865403e0f",
    )

    http_archive(
        name = "rules_cc",
        sha256 = "a2fdfde2ab9b2176bd6a33afca14458039023edb1dd2e73e6823810809df4027",
        strip_prefix = "rules_cc-0.2.14",
        urls = ["https://github.com/bazelbuild/rules_cc/releases/download/0.2.14/rules_cc-0.2.14.tar.gz"],
    )

    # Using sorbet/bazel-toolchain fork - needs to be updated for Bazel 8 compatibility
    http_archive(
        name = "toolchains_llvm",
        url = "https://github.com/sorbet/bazel-toolchain/archive/5ed6d56dd7d2466bda56a6237c5ed70336b95ee5.tar.gz",
        sha256 = "bdc706dbc33811ce4b2089d52564da106c2afbf3723cffbef301cc64e7615251",
        strip_prefix = "bazel-toolchain-5ed6d56dd7d2466bda56a6237c5ed70336b95ee5",
    )

    http_archive(
        name = "com_github_bazelbuild_buildtools",
        url = "https://github.com/bazelbuild/buildtools/archive/5bcc31df55ec1de770cb52887f2e989e7068301f.zip",
        sha256 = "875d0c49953e221cfc35d2a3846e502f366dfa4024b271fa266b186ca4664b37",
        strip_prefix = "buildtools-5bcc31df55ec1de770cb52887f2e989e7068301f",
    )

    # optimized version of blake2 hashing algorithm, using SSE vector extensions
    http_archive(
        name = "com_github_blake2_libb2",
        url = "https://github.com/BLAKE2/libb2/archive/fa83ddbe179912e9a7a57edf0333b33f6ff83056.zip",
        sha256 = "dd25f7ac53371c2a15761fc1689d04de2ff948ac7e213a10d13961e24b0c9ae6",
        build_file = "@com_stripe_ruby_typer//third_party:libb2.BUILD",
        strip_prefix = "libb2-fa83ddbe179912e9a7a57edf0333b33f6ff83056",
    )

    # portable reference implementation of blake2
    http_archive(
        name = "com_github_blake2_blake2",
        url = "https://github.com/BLAKE2/BLAKE2/archive/997fa5ba1e14b52c554fb03ce39e579e6f27b90c.zip",
        sha256 = "56dafe9512f65728ce7abc78900272f8bf8e95ca04439b362d2dc461927b2a17",
        build_file = "@com_stripe_ruby_typer//third_party:blake2.BUILD",
        strip_prefix = "BLAKE2-997fa5ba1e14b52c554fb03ce39e579e6f27b90c",
    )

    # mpack now comes from MODULE.bazel with archive_override

    # crcpp now comes from MODULE.bazel with archive_override

    http_archive(
        name = "emsdk",
        sha256 = "47515d522229a103b7d9f34eacc1d88ac355b22fd754d13417a2191fd9d77d5f",
        strip_prefix = "emsdk-3.1.59/bazel",
        url = "https://github.com/emscripten-core/emsdk/archive/3.1.59.tar.gz",
    )

    http_archive(
        name = "rules_ragel",
        url = "https://github.com/jmillikin/rules_ragel/archive/f99f17fcad2e155646745f4827ac636a3b5d4d15.zip",
        sha256 = "f957682c6350b2e4484c433c7f45d427a86de5c8751a0d2a9836f36995fe0320",
        strip_prefix = "rules_ragel-f99f17fcad2e155646745f4827ac636a3b5d4d15",
    )

    http_archive(
        name = "rules_bison",
        url = "https://github.com/jmillikin/rules_bison/archive/478079b28605a38000eaf83719568d756b3383a0.zip",
        sha256 = "d662d200f4e2a868f6873d666402fa4d413f07ba1a433591c5f60ac601157fb9",
        strip_prefix = "rules_bison-478079b28605a38000eaf83719568d756b3383a0",
    )

    http_archive(
        name = "rules_m4",
        url = "https://github.com/jmillikin/rules_m4/releases/download/v0.2.1/rules_m4-v0.2.1.tar.xz",
        sha256 = "f59f75ac8a315d7647a2d058d324a87ff9ebbc4bf5c7a61b08d58da119a7fb43",
    )

    # cpp_subprocess now comes from MODULE.bazel with archive_override

    shellcheck_version = "0.8.0"
    http_archive(
        name = "shellcheck_linux",
        url = "https://github.com/koalaman/shellcheck/releases/download/v{0}/shellcheck-v{0}.linux.x86_64.tar.xz".format(shellcheck_version),
        build_file = "@com_stripe_ruby_typer//third_party:shellcheck.BUILD",
        sha256 = "ab6ee1b178f014d1b86d1e24da20d1139656c8b0ed34d2867fbb834dad02bf0a",
        strip_prefix = "shellcheck-v{}".format(shellcheck_version),
    )
    http_archive(
        name = "shellcheck_darwin",
        url = "https://github.com/koalaman/shellcheck/releases/download/v{0}/shellcheck-v{0}.darwin.x86_64.tar.xz".format(shellcheck_version),
        build_file = "@com_stripe_ruby_typer//third_party:shellcheck.BUILD",
        sha256 = "e065d4afb2620cc8c1d420a9b3e6243c84ff1a693c1ff0e38f279c8f31e86634",
        strip_prefix = "shellcheck-v{}".format(shellcheck_version),
    )

    http_archive(
        name = "rules_foreign_cc",
        url = "https://github.com/bazelbuild/rules_foreign_cc/archive/d74623f0ad47f4e375de81baa454eb106715a416.zip",
        sha256 = "47b61d25dd52bdaa1d571dab6705d076f05ba3d7a1bbbfed36145f8281c0403f",
        strip_prefix = "rules_foreign_cc-d74623f0ad47f4e375de81baa454eb106715a416",
    )

    http_archive(
        name = "rbs_parser",
        url = "https://github.com/ruby/rbs/archive/e5901cd788caa00392c38518e27d3c2800a34328.zip",
        sha256 = "09413ce9f75e7f65fdafb6c64a2feed20f12c5c0e1b383716fd4f2fe92eae1a0",
        strip_prefix = "rbs-e5901cd788caa00392c38518e27d3c2800a34328",
        build_file = "@com_stripe_ruby_typer//third_party:rbs_parser.BUILD",
    )
