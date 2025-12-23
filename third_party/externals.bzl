load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Toolchain dependencies that must stay in WORKSPACE.
# Library and rules dependencies have been migrated to MODULE.bazel.
def register_sorbet_dependencies():
    http_archive(
        name = "com_grail_bazel_compdb",
        url = "https://github.com/grailbio/bazel-compilation-database/archive/6b9329e37295eab431f82af5fe24219865403e0f.zip",
        sha256 = "6cf0dc4b40023a26787cd7cdb629dccd26e2208c8a2f19e1dde4ca10c109c86c",
        strip_prefix = "bazel-compilation-database-6b9329e37295eab431f82af5fe24219865403e0f",
    )

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

    http_archive(
        name = "rules_foreign_cc",
        url = "https://github.com/bazelbuild/rules_foreign_cc/archive/d74623f0ad47f4e375de81baa454eb106715a416.zip",
        sha256 = "47b61d25dd52bdaa1d571dab6705d076f05ba3d7a1bbbfed36145f8281c0403f",
        strip_prefix = "rules_foreign_cc-d74623f0ad47f4e375de81baa454eb106715a416",
    )

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
