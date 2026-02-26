load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# We define our externals here instead of directly in WORKSPACE
def register_sorbet_dependencies():
    http_archive(
        name = "platforms",
        url = "https://github.com/bazelbuild/platforms/releases/download/0.0.10/platforms-0.0.10.tar.gz",
        sha256 = "218efe8ee736d26a3572663b374a253c012b716d8af0c07e842e82f238a0a7ee",
    )

    http_archive(
        name = "prism",
        url = "https://github.com/ruby/prism/releases/download/v1.6.0/libprism-src.tar.gz",
        sha256 = "a643517b910c510c998a518e45a66f7f3460e5b32f80d64aa0021fed7c967d0f",
        build_file = "@com_stripe_ruby_typer//third_party:prism.BUILD",
        strip_prefix = "libprism-src",
    )

    http_archive(
        name = "doctest",
        url = "https://github.com/doctest/doctest/archive/v2.4.9.zip",
        sha256 = "88a552f832ef3e4e7b733f9ab4eff5d73d7c37e75bebfef4a3339bf52713350d",
        strip_prefix = "doctest-2.4.9",
    )

    http_archive(
        name = "dtl",
        url = "https://github.com/cubicdaiya/dtl/archive/v1.19.tar.gz",
        sha256 = "f47b99dd11e5d771ad32a8dc960db4ab2fbe349fb0346fa0795f53c846a99c5d",
        build_file = "@com_stripe_ruby_typer//third_party:dtl.BUILD",
        strip_prefix = "dtl-1.19",
    )

    http_archive(
        name = "yaml_cpp",
        url = "https://github.com/jbeder/yaml-cpp/archive/yaml-cpp-0.6.3.zip",
        sha256 = "7c0ddc08a99655508ae110ba48726c67e4a10b290c214aed866ce4bbcbe3e84c",
        build_file = "@com_stripe_ruby_typer//third_party:yaml_cpp.BUILD",
        strip_prefix = "yaml-cpp-yaml-cpp-0.6.3",
    )

    http_archive(
        name = "spdlog",
        url = "https://github.com/gabime/spdlog/archive/8e5613379f5140fefb0b60412fbf1f5406e7c7f8.zip",  # v1.15.0
        sha256 = "86d0688c088f6cad36533c731e8377882d1cb0d05508afa3e624d3c0e7cf92af",
        build_file = "@com_stripe_ruby_typer//third_party:spdlog.BUILD",
        strip_prefix = "spdlog-8e5613379f5140fefb0b60412fbf1f5406e7c7f8",
    )

    # We don't use this directly, but protobuf will skip defining its own
    # `@zlib` if it's present.
    http_archive(
        name = "zlib",
        url = "https://github.com/madler/zlib/archive/v1.3.1.zip",
        build_file = "@com_stripe_ruby_typer//third_party:zlib.BUILD",
        sha256 = "50b24b47bf19e1f35d2a21ff36d2a366638cdf958219a66f30ce0861201760e6",
        strip_prefix = "zlib-1.3.1",
    )

    # proto_library, cc_proto_library, and java_proto_library rules implicitly
    # depend on @com_google_protobuf for protoc and proto runtimes.
    # This statement defines the @com_google_protobuf repo.
    http_archive(
        name = "com_google_protobuf",
        url = "https://github.com/protocolbuffers/protobuf/archive/v3.27.0.zip",
        sha256 = "913530eba097b17f58b9087fe9c4944de87b56913e3e340b91e317d1e6763dde",
        strip_prefix = "protobuf-3.27.0",
        patches = [
            "@com_stripe_ruby_typer//third_party:com_google_protobuf/cpp_opts.bzl.patch",
        ],
    )

    http_archive(
        name = "libprotobuf-mutator",
        url = "https://github.com/google/libprotobuf-mutator/archive/68e10c13248517c5bcd531d0e02be483da83fc13.zip",
        sha256 = "8684276b996e8d8541ed3703420f9dbcc17702bd7b13c6f3d9c13a4656597c76",
        build_file = "@com_stripe_ruby_typer//third_party:libprotobuf-mutator.BUILD",
        strip_prefix = "libprotobuf-mutator-68e10c13248517c5bcd531d0e02be483da83fc13",
    )

    http_archive(
        name = "lmdb",
        url = "https://github.com/LMDB/lmdb/archive/da9aeda08c3ff710a0d47d61a079f5a905b0a10a.zip",
        sha256 = "893a324b60c6465edcf7feb7dafacb3d5f5803649f1b5de6f453b3a6d9e2bb97",
        build_file = "@com_stripe_ruby_typer//third_party:lmdb.BUILD",
        strip_prefix = "lmdb-da9aeda08c3ff710a0d47d61a079f5a905b0a10a",
        patches = [
            "@com_stripe_ruby_typer//third_party:lmdb/strdup.patch",
        ],
    )

    http_archive(
        name = "rapidjson",
        url = "https://github.com/Tencent/rapidjson/archive/f376690822cbc2d17044e626be5df21f7d91ca8f.zip",
        sha256 = "9425276583dff9020cee6332472b0cf247ae325cb5f26dbe157183f747da3910",
        build_file = "@com_stripe_ruby_typer//third_party:rapidjson.BUILD",
        strip_prefix = "rapidjson-f376690822cbc2d17044e626be5df21f7d91ca8f",
    )

    http_archive(
        name = "lz4",
        url = "https://github.com/lz4/lz4/archive/v1.9.3.zip",
        sha256 = "4ec935d99aa4950eadfefbd49c9fad863185ac24c32001162c44a683ef61b580",
        build_file = "@com_stripe_ruby_typer//third_party:lz4.BUILD",
        strip_prefix = "lz4-1.9.3",
    )

    http_archive(
        name = "pdqsort",
        url = "https://github.com/orlp/pdqsort/archive/08879029ab8dcb80a70142acb709e3df02de5d37.zip",
        sha256 = "ad8c9cd3d1abe5d566bad341bcce327a2e897b64236a7f9e74f4b9b0e7e5dc39",
        build_file = "@com_stripe_ruby_typer//third_party:pdqsort.BUILD",
        strip_prefix = "pdqsort-08879029ab8dcb80a70142acb709e3df02de5d37",
    )

    http_archive(
        name = "jemalloc",
        url = "https://github.com/jemalloc/jemalloc/archive/20f9802e4f25922884448d9581c66d76cc905c0c.zip",  # 5.3
        sha256 = "1cc1ec93701868691c73b371eb87e5452257996279a42303a91caad355374439",
        build_file = "@com_stripe_ruby_typer//third_party:jemalloc.BUILD",
        strip_prefix = "jemalloc-20f9802e4f25922884448d9581c66d76cc905c0c",
    )

    http_archive(
        name = "mimalloc",
        url = "https://github.com/microsoft/mimalloc/archive/refs/tags/v2.1.2.zip",  # 2.1.2
        sha256 = "86281c918921c1007945a8a31e5ad6ae9af77e510abfec20d000dd05d15123c7",
        build_file = "@com_stripe_ruby_typer//third_party:mimalloc.BUILD",
        strip_prefix = "mimalloc-2.1.2",
    )

    http_archive(
        name = "concurrentqueue",
        url = "https://github.com/cameron314/concurrentqueue/archive/79cec4c3bf1ca23ea4a03adfcd3c2c3659684dd2.zip",
        sha256 = "a78ff232e2996927ad6fbd015d1f15dfb20bf524a87ce2893e64dbbe1f04051e",
        build_file = "@com_stripe_ruby_typer//third_party:concurrentqueue.BUILD",
        strip_prefix = "concurrentqueue-79cec4c3bf1ca23ea4a03adfcd3c2c3659684dd2",
    )

    http_archive(
        name = "statsd",
        url = "https://github.com/romanbsd/statsd-c-client/archive/08ecca678345f157e72a1db1446facb403cbeb65.zip",
        sha256 = "825395556fb553383173e47dbce98165981d100587993292ec9d174ec40a7ba1",
        build_file = "@com_stripe_ruby_typer//third_party:statsd.BUILD",
        strip_prefix = "statsd-c-client-08ecca678345f157e72a1db1446facb403cbeb65",
    )

    http_archive(
        name = "cxxopts",
        url = "https://github.com/jarro2783/cxxopts/archive/c74846a891b3cc3bfa992d588b1295f528d43039.zip",
        sha256 = "4ba2b0a3c94e61501b974118a0fe171cd658f8efdd941e9ad82e71f48a98933a",
        build_file = "@com_stripe_ruby_typer//third_party:cxxopts.BUILD",
        strip_prefix = "cxxopts-c74846a891b3cc3bfa992d588b1295f528d43039",
    )

    http_archive(
        name = "rang",
        url = "https://github.com/agauniyal/rang/archive/v3.1.0.zip",
        sha256 = "658adeb8a36d36981d4339fc839f2deedc0e75cb421db1982041d8a0a255835d",
        build_file = "@com_stripe_ruby_typer//third_party:rang.BUILD",
        strip_prefix = "rang-3.1.0",
    )

    http_archive(
        name = "xxhash",
        url = "https://github.com/Cyan4973/xxHash/archive/v0.8.0.zip",
        sha256 = "064333c754f166837bbefefa497642a60b3f8035e54bae52eb304d3cb3ceb655",
        build_file = "@com_stripe_ruby_typer//third_party:xxhash.BUILD",
        strip_prefix = "xxHash-0.8.0",
    )

    http_archive(
        name = "com_google_absl",
        url = "https://github.com/abseil/abseil-cpp/archive/20240722.0.zip",
        sha256 = "95e90be7c3643e658670e0dd3c1b27092349c34b632c6e795686355f67eca89f",
        strip_prefix = "abseil-cpp-20240722.0",
    )

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
        urls = ["https://github.com/bazelbuild/rules_cc/archive/refs/tags/0.2.14.tar.gz"],
    )

    # TODO(jez) We keep our changes on the `sorbet` branch of `sorbet/bazel-toolchain`
    # The `master` branch is the commit of `bazel-contrib/toolchains_llvm` that we're based on
    # In 2ddd7d791 (#7912) we upgraded the toolchain. Our old toolchain patches are on the `sorbet-old-toolchain` branch
    #
    # You can use this version of `toolchains_llvm` when tinkering locally. You'll want to run `bazel clean --expunge`
    # to ensure that your changes get picked up between builds.
    # native.local_repository(
    #     name = "toolchains_llvm",
    #     path = "../bazel-toolchain",
    # )
    http_archive(
        name = "toolchains_llvm",
        url = "https://github.com/sorbet/bazel-toolchain/archive/b9c2f11092c90377d4bfc64c406766573545af47.tar.gz",
        sha256 = "723ebe8994d754acb3ef81a271ad645a0a3ee4a8680f0499b3c24f537b16cfcc",
        strip_prefix = "bazel-toolchain-b9c2f11092c90377d4bfc64c406766573545af47",
    )

    http_archive(
        name = "io_bazel_rules_go",
        sha256 = "d6ab6b57e48c09523e93050f13698f708428cfd5e619252e369d377af6597707",
        url = "https://github.com/bazelbuild/rules_go/releases/download/v0.43.0/rules_go-v0.43.0.zip",
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

    http_archive(
        name = "com_github_ludocode_mpack",
        url = "https://github.com/ludocode/mpack/archive/6883f6f8067d5c6dbaaddeaa47aaaaa763eea51c.zip",
        sha256 = "fb184dc169722cecf9b47bece308f70861787f4615ebdbee7383b6434cfdbc0d",
        build_file = "@com_stripe_ruby_typer//third_party:mpack.BUILD",
        strip_prefix = "mpack-6883f6f8067d5c6dbaaddeaa47aaaaa763eea51c",
    )

    http_archive(
        name = "com_github_d_bahr_crcpp",
        url = "https://github.com/d-bahr/CRCpp/archive/51fbc35ef892e98abe91a51f7320749c929d72bd.zip",
        sha256 = "57c4c127b5aa4451556969d6929cf9465a5d5481b3442ddb878d95296caeee4b",
        build_file = "@com_stripe_ruby_typer//third_party:crcpp.BUILD",
        strip_prefix = "CRCpp-51fbc35ef892e98abe91a51f7320749c929d72bd",
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

    http_archive(
        name = "cpp_subprocess",
        url = "https://github.com/arun11299/cpp-subprocess/archive/9c624ce4e3423cce9f148bafbae56abfd6437ea0.zip",
        sha256 = "1810d1ec80f3c319dcbb530443b264b9a32a449b5a5d3630076e473648bba8cc",
        build_file = "@com_stripe_ruby_typer//third_party:cpp_subprocess.BUILD",
        strip_prefix = "cpp-subprocess-9c624ce4e3423cce9f148bafbae56abfd6437ea0",
    )

    http_archive(
        name = "bazel_skylib",
        sha256 = "cd55a062e763b9349921f0f5db8c3933288dc8ba4f76dd9416aac68acee3cb94",
        url = "https://github.com/bazelbuild/bazel-skylib/releases/download/1.5.0/bazel-skylib-1.5.0.tar.gz",
    )

    http_archive(
        name = "aspect_bazel_lib",
        sha256 = "357dad9d212327c35d9244190ef010aad315e73ffa1bed1a29e20c372f9ca346",
        strip_prefix = "bazel-lib-2.7.0",
        url = "https://github.com/aspect-build/bazel-lib/releases/download/v2.7.0/bazel-lib-v2.7.0.tar.gz",
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

    # Needed to build CMake projects
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
