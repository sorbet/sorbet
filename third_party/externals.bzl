load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//third_party:ruby_externals.bzl", "register_ruby_dependencies")
load("//third_party/openssl:system_openssl_repository.bzl", "system_openssl_repository")

# We define our externals here instead of directly in WORKSPACE
def register_sorbet_dependencies():
    http_archive(
        name = "platforms",
        urls = _github_public_urls("bazelbuild/platforms/archive/d4c9d7f51a7c403814b60f66d20eeb425fbaaacb.zip"),
        sha256 = "a5058ac93023092c406432ec650f30ec5a8c75d8b9d13c73150f60a9050a5663",
        strip_prefix = "platforms-d4c9d7f51a7c403814b60f66d20eeb425fbaaacb",
    )

    http_archive(
        name = "doctest",
        urls = _github_public_urls("doctest/doctest/archive/v2.4.9.zip"),
        sha256 = "88a552f832ef3e4e7b733f9ab4eff5d73d7c37e75bebfef4a3339bf52713350d",
        strip_prefix = "doctest-2.4.9",
    )

    http_archive(
        name = "dtl",
        urls = _github_public_urls("cubicdaiya/dtl/archive/v1.19.tar.gz"),
        sha256 = "f47b99dd11e5d771ad32a8dc960db4ab2fbe349fb0346fa0795f53c846a99c5d",
        build_file = "@com_stripe_ruby_typer//third_party:dtl.BUILD",
        strip_prefix = "dtl-1.19",
    )

    http_archive(
        name = "yaml_cpp",
        urls = _github_public_urls("jbeder/yaml-cpp/archive/yaml-cpp-0.6.3.zip"),
        sha256 = "7c0ddc08a99655508ae110ba48726c67e4a10b290c214aed866ce4bbcbe3e84c",
        build_file = "@com_stripe_ruby_typer//third_party:yaml_cpp.BUILD",
        strip_prefix = "yaml-cpp-yaml-cpp-0.6.3",
    )

    http_archive(
        name = "spdlog",
        urls = _github_public_urls("gabime/spdlog/archive/eb3220622e73a4889eee355ffa37972b3cac3df5.zip"),  # v1.9.2
        sha256 = "b7570488bdd94ab6d3653bc324d8ed7976d9f3a2f035eb2e969ebcaad3b0d5c7",
        build_file = "@com_stripe_ruby_typer//third_party:spdlog.BUILD",
        strip_prefix = "spdlog-eb3220622e73a4889eee355ffa37972b3cac3df5",
    )

    # We don't use this directly, but protobuf will skip defining its own
    # `@zlib` if it's present.
    http_archive(
        name = "zlib",
        urls = _github_public_urls("madler/zlib/archive/cacf7f1d4e3d44d871b605da3b647f07d718623f.zip"),
        build_file = "@com_stripe_ruby_typer//third_party:zlib.BUILD",
        sha256 = "1cce3828ec2ba80ff8a4cac0ab5aa03756026517154c4b450e617ede751d41bd",
        strip_prefix = "zlib-cacf7f1d4e3d44d871b605da3b647f07d718623f",
    )

    # proto_library, cc_proto_library, and java_proto_library rules implicitly
    # depend on @com_google_protobuf for protoc and proto runtimes.
    # This statement defines the @com_google_protobuf repo.
    http_archive(
        name = "com_google_protobuf",
        urls = _github_public_urls("protocolbuffers/protobuf/archive/v3.14.0.zip"),
        sha256 = "bf0e5070b4b99240183b29df78155eee335885e53a8af8683964579c214ad301",
        strip_prefix = "protobuf-3.14.0",
    )

    http_archive(
        name = "libprotobuf-mutator",
        urls = _github_public_urls("google/libprotobuf-mutator/archive/68e10c13248517c5bcd531d0e02be483da83fc13.zip"),
        sha256 = "8684276b996e8d8541ed3703420f9dbcc17702bd7b13c6f3d9c13a4656597c76",
        build_file = "@com_stripe_ruby_typer//third_party:libprotobuf-mutator.BUILD",
        strip_prefix = "libprotobuf-mutator-68e10c13248517c5bcd531d0e02be483da83fc13",
    )

    http_archive(
        name = "lmdb",
        urls = _github_public_urls("DarkDimius/lmdb/archive/75766ec2b663b360be8eea9730a7adc0d252ce7e.zip"),
        sha256 = "bd120470d62c6f3433f80bb9841f09f158924081eb0c3236da6e8d1a0976eccc",
        build_file = "@com_stripe_ruby_typer//third_party:lmdb.BUILD",
        strip_prefix = "lmdb-75766ec2b663b360be8eea9730a7adc0d252ce7e",
    )

    http_archive(
        name = "rapidjson",
        urls = _github_public_urls("Tencent/rapidjson/archive/f376690822cbc2d17044e626be5df21f7d91ca8f.zip"),
        sha256 = "9425276583dff9020cee6332472b0cf247ae325cb5f26dbe157183f747da3910",
        build_file = "@com_stripe_ruby_typer//third_party:rapidjson.BUILD",
        strip_prefix = "rapidjson-f376690822cbc2d17044e626be5df21f7d91ca8f",
    )

    http_archive(
        name = "lz4",
        urls = _github_public_urls("lz4/lz4/archive/v1.9.3.zip"),
        sha256 = "4ec935d99aa4950eadfefbd49c9fad863185ac24c32001162c44a683ef61b580",
        build_file = "@com_stripe_ruby_typer//third_party:lz4.BUILD",
        strip_prefix = "lz4-1.9.3",
    )

    http_archive(
        name = "pdqsort",
        urls = _github_public_urls("orlp/pdqsort/archive/08879029ab8dcb80a70142acb709e3df02de5d37.zip"),
        sha256 = "ad8c9cd3d1abe5d566bad341bcce327a2e897b64236a7f9e74f4b9b0e7e5dc39",
        build_file = "@com_stripe_ruby_typer//third_party:pdqsort.BUILD",
        strip_prefix = "pdqsort-08879029ab8dcb80a70142acb709e3df02de5d37",
    )

    http_archive(
        name = "jemalloc",
        urls = _github_public_urls("jemalloc/jemalloc/archive/20f9802e4f25922884448d9581c66d76cc905c0c.zip"),  # 5.3
        sha256 = "1cc1ec93701868691c73b371eb87e5452257996279a42303a91caad355374439",
        build_file = "@com_stripe_ruby_typer//third_party:jemalloc.BUILD",
        strip_prefix = "jemalloc-20f9802e4f25922884448d9581c66d76cc905c0c",
    )

    http_archive(
        name = "mimalloc",
        urls = _github_public_urls("microsoft/mimalloc/archive/refs/tags/v2.1.2.zip"),  # 2.1.2
        sha256 = "86281c918921c1007945a8a31e5ad6ae9af77e510abfec20d000dd05d15123c7",
        build_file = "@com_stripe_ruby_typer//third_party:mimalloc.BUILD",
        strip_prefix = "mimalloc-2.1.2",
    )

    http_archive(
        name = "concurrentqueue",
        urls = _github_public_urls("cameron314/concurrentqueue/archive/79cec4c3bf1ca23ea4a03adfcd3c2c3659684dd2.zip"),
        sha256 = "a78ff232e2996927ad6fbd015d1f15dfb20bf524a87ce2893e64dbbe1f04051e",
        build_file = "@com_stripe_ruby_typer//third_party:concurrentqueue.BUILD",
        strip_prefix = "concurrentqueue-79cec4c3bf1ca23ea4a03adfcd3c2c3659684dd2",
    )

    http_archive(
        name = "statsd",
        urls = _github_public_urls("romanbsd/statsd-c-client/archive/08ecca678345f157e72a1db1446facb403cbeb65.zip"),
        sha256 = "825395556fb553383173e47dbce98165981d100587993292ec9d174ec40a7ba1",
        build_file = "@com_stripe_ruby_typer//third_party:statsd.BUILD",
        strip_prefix = "statsd-c-client-08ecca678345f157e72a1db1446facb403cbeb65",
    )

    http_archive(
        name = "cxxopts",
        urls = _github_public_urls("jarro2783/cxxopts/archive/c74846a891b3cc3bfa992d588b1295f528d43039.zip"),
        sha256 = "4ba2b0a3c94e61501b974118a0fe171cd658f8efdd941e9ad82e71f48a98933a",
        build_file = "@com_stripe_ruby_typer//third_party:cxxopts.BUILD",
        strip_prefix = "cxxopts-c74846a891b3cc3bfa992d588b1295f528d43039",
    )

    http_archive(
        name = "rang",
        urls = _github_public_urls("agauniyal/rang/archive/v3.1.0.zip"),
        sha256 = "658adeb8a36d36981d4339fc839f2deedc0e75cb421db1982041d8a0a255835d",
        build_file = "@com_stripe_ruby_typer//third_party:rang.BUILD",
        strip_prefix = "rang-3.1.0",
    )

    http_archive(
        name = "xxhash",
        urls = _github_public_urls("Cyan4973/xxHash/archive/v0.8.0.zip"),
        sha256 = "064333c754f166837bbefefa497642a60b3f8035e54bae52eb304d3cb3ceb655",
        build_file = "@com_stripe_ruby_typer//third_party:xxhash.BUILD",
        strip_prefix = "xxHash-0.8.0",
    )

    http_archive(
        name = "com_google_absl",
        urls = _github_public_urls("abseil/abseil-cpp/archive/8910297baf87e1777c4fd30fb0693eecf9f2c134.zip"),
        sha256 = "c43b8cd8e306e7fe3f006d880181d60db59a3bae6b6bc725da86a28a6b0f9f30",
        strip_prefix = "abseil-cpp-8910297baf87e1777c4fd30fb0693eecf9f2c134",
    )

    http_archive(
        name = "com_grail_bazel_compdb",
        urls = _github_public_urls("grailbio/bazel-compilation-database/archive/6b9329e37295eab431f82af5fe24219865403e0f.zip"),
        sha256 = "6cf0dc4b40023a26787cd7cdb629dccd26e2208c8a2f19e1dde4ca10c109c86c",
        strip_prefix = "bazel-compilation-database-6b9329e37295eab431f82af5fe24219865403e0f",
    )

    http_archive(
        name = "rules_cc",
        sha256 = "b6f34b3261ec02f85dbc5a8bdc9414ce548e1f5f67e000d7069571799cb88b25",
        strip_prefix = "rules_cc-726dd8157557f1456b3656e26ab21a1646653405",
        urls = ["https://github.com/bazelbuild/rules_cc/archive/726dd8157557f1456b3656e26ab21a1646653405.tar.gz"],
    )

    # NOTE: we use the sorbet branch for development to keep our changes rebasable on grailio/bazel-toolchain
    http_archive(
        name = "com_grail_bazel_toolchain",
        urls = _github_public_urls("sorbet/bazel-toolchain/archive/5c23c6f550bc2a1db0d5d0b9cf83669a1f91d46e.zip"),
        sha256 = "53c76d48dfa63c00961293fb73843fe7d9bee647e39f98c7d6a04e256eb40514",
        strip_prefix = "bazel-toolchain-5c23c6f550bc2a1db0d5d0b9cf83669a1f91d46e",
    )

    http_archive(
        name = "io_bazel_rules_go",
        sha256 = "d6ab6b57e48c09523e93050f13698f708428cfd5e619252e369d377af6597707",
        urls = _github_public_urls("bazelbuild/rules_go/releases/download/v0.43.0/rules_go-v0.43.0.zip"),
    )

    http_archive(
        name = "build_bazel_rules_nodejs",
        sha256 = "e79c08a488cc5ac40981987d862c7320cee8741122a2649e9b08e850b6f20442",
        urls = _github_public_urls("bazelbuild/rules_nodejs/releases/download/3.8.0/rules_nodejs-3.8.0.tar.gz"),
    )

    http_archive(
        name = "com_github_bazelbuild_buildtools",
        urls = _github_public_urls("bazelbuild/buildtools/archive/5bcc31df55ec1de770cb52887f2e989e7068301f.zip"),
        sha256 = "875d0c49953e221cfc35d2a3846e502f366dfa4024b271fa266b186ca4664b37",
        strip_prefix = "buildtools-5bcc31df55ec1de770cb52887f2e989e7068301f",
    )

    # optimized version of blake2 hashing algorithm
    http_archive(
        name = "com_github_blake2_libb2",
        urls = _github_public_urls("BLAKE2/libb2/archive/fa83ddbe179912e9a7a57edf0333b33f6ff83056.zip"),
        sha256 = "dd25f7ac53371c2a15761fc1689d04de2ff948ac7e213a10d13961e24b0c9ae6",
        build_file = "@com_stripe_ruby_typer//third_party:libb2.BUILD",
        strip_prefix = "libb2-fa83ddbe179912e9a7a57edf0333b33f6ff83056",
    )

    # portable reference implementation of blake2
    http_archive(
        name = "com_github_blake2_blake2",
        urls = _github_public_urls("BLAKE2/BLAKE2/archive/997fa5ba1e14b52c554fb03ce39e579e6f27b90c.zip"),
        sha256 = "56dafe9512f65728ce7abc78900272f8bf8e95ca04439b362d2dc461927b2a17",
        build_file = "@com_stripe_ruby_typer//third_party:blake2.BUILD",
        strip_prefix = "BLAKE2-997fa5ba1e14b52c554fb03ce39e579e6f27b90c",
    )

    http_archive(
        name = "com_github_ludocode_mpack",
        urls = _github_public_urls("ludocode/mpack/archive/6883f6f8067d5c6dbaaddeaa47aaaaa763eea51c.zip"),
        sha256 = "fb184dc169722cecf9b47bece308f70861787f4615ebdbee7383b6434cfdbc0d",
        build_file = "@com_stripe_ruby_typer//third_party:mpack.BUILD",
        strip_prefix = "mpack-6883f6f8067d5c6dbaaddeaa47aaaaa763eea51c",
    )

    http_archive(
        name = "com_github_d_bahr_crcpp",
        urls = _github_public_urls("d-bahr/CRCpp/archive/51fbc35ef892e98abe91a51f7320749c929d72bd.zip"),
        sha256 = "57c4c127b5aa4451556969d6929cf9465a5d5481b3442ddb878d95296caeee4b",
        build_file = "@com_stripe_ruby_typer//third_party:crcpp.BUILD",
        strip_prefix = "CRCpp-51fbc35ef892e98abe91a51f7320749c929d72bd",
    )

    http_archive(
        name = "emscripten_toolchain",
        urls = _github_public_urls("kripken/emscripten/archive/1.38.25.tar.gz"),
        build_file = "@com_stripe_ruby_typer//third_party:emscripten-toolchain.BUILD",
        sha256 = "4d6fa350895fabc25b89ce5f9dcb528e719e7c2bf7dacab2a3e3cc818ecd7019",
        strip_prefix = "emscripten-1.38.25",
        patches = [
            "@com_stripe_ruby_typer//third_party:emscripten_toolchain/emcc.py.patch",
            "@com_stripe_ruby_typer//third_party:emscripten_toolchain/tools_shared.py.patch",
        ],
    )

    http_archive(
        name = "emscripten_clang_linux",
        urls = _emscripten_urls("linux/emscripten-llvm-e1.38.25.tar.gz"),
        build_file = "@com_stripe_ruby_typer//third_party:emscripten-clang.BUILD",
        sha256 = "0e9a5a114a60c21604f4038b573109bd31424aeba275b4173480485ca0a56ba4",
        strip_prefix = "emscripten-llvm-e1.38.25",
    )

    http_archive(
        name = "emscripten_clang_darwin",
        urls = _emscripten_urls("mac/emscripten-llvm-e1.38.25.tar.gz"),
        build_file = "@com_stripe_ruby_typer//third_party:emscripten-clang.BUILD",
        sha256 = "01519125c613d0b013193eaf5ac5031e6ec34aac2451c357fd4097874ceee38c",
        strip_prefix = "emscripten-llvm-e1.38.25",
    )

    http_archive(
        name = "rules_ragel",
        urls = _github_public_urls("jmillikin/rules_ragel/archive/f99f17fcad2e155646745f4827ac636a3b5d4d15.zip"),
        sha256 = "f957682c6350b2e4484c433c7f45d427a86de5c8751a0d2a9836f36995fe0320",
        strip_prefix = "rules_ragel-f99f17fcad2e155646745f4827ac636a3b5d4d15",
    )

    http_archive(
        name = "rules_bison",
        urls = _github_public_urls("jmillikin/rules_bison/archive/478079b28605a38000eaf83719568d756b3383a0.zip"),
        sha256 = "d662d200f4e2a868f6873d666402fa4d413f07ba1a433591c5f60ac601157fb9",
        strip_prefix = "rules_bison-478079b28605a38000eaf83719568d756b3383a0",
    )

    http_archive(
        name = "rules_m4",
        urls = _github_public_urls("jmillikin/rules_m4/releases/download/v0.2.1/rules_m4-v0.2.1.tar.xz"),
        sha256 = "f59f75ac8a315d7647a2d058d324a87ff9ebbc4bf5c7a61b08d58da119a7fb43",
    )

    http_archive(
        name = "cpp_subprocess",
        urls = _github_public_urls("arun11299/cpp-subprocess/archive/9c624ce4e3423cce9f148bafbae56abfd6437ea0.zip"),
        sha256 = "1810d1ec80f3c319dcbb530443b264b9a32a449b5a5d3630076e473648bba8cc",
        build_file = "@com_stripe_ruby_typer//third_party:cpp_subprocess.BUILD",
        strip_prefix = "cpp-subprocess-9c624ce4e3423cce9f148bafbae56abfd6437ea0",
    )

    system_openssl_repository(
        name = "system_ssl_darwin",
        build_file = "@com_stripe_ruby_typer//third_party/openssl:darwin.BUILD",
        openssl_dirs = [
            "/usr/local/opt/openssl@1.1",
            "/opt/homebrew/opt/openssl@1.1",
            "/usr/local/opt/openssl",
            "/opt/homebrew/opt/openssl",
        ],
    )

    # If we ever want to search multiple paths, we can likely use the
    # `system_openssl_repository` repository rule like above. But I figure that
    # right now if it ain't broke don't fix it, so I've left this using
    # new_local_repository.
    native.new_local_repository(
        name = "system_ssl_linux",
        path = "/usr",
        build_file = "@com_stripe_ruby_typer//third_party/openssl:linux.BUILD",
    )

    http_archive(
        name = "bazel_skylib",
        sha256 = "cd55a062e763b9349921f0f5db8c3933288dc8ba4f76dd9416aac68acee3cb94",
        urls = _github_public_urls("bazelbuild/bazel-skylib/releases/download/1.5.0/bazel-skylib-1.5.0.tar.gz"),
    )

    http_archive(
        name = "aspect_bazel_lib",
        sha256 = "357dad9d212327c35d9244190ef010aad315e73ffa1bed1a29e20c372f9ca346",
        strip_prefix = "bazel-lib-2.7.0",
        urls = _github_public_urls("aspect-build/bazel-lib/releases/download/v2.7.0/bazel-lib-v2.7.0.tar.gz"),
    )

    shellcheck_version = "0.8.0"
    http_archive(
        name = "shellcheck_linux",
        urls = _github_public_urls("koalaman/shellcheck/releases/download/v{0}/shellcheck-v{0}.linux.x86_64.tar.xz".format(shellcheck_version)),
        build_file = "@com_stripe_ruby_typer//third_party:shellcheck.BUILD",
        sha256 = "ab6ee1b178f014d1b86d1e24da20d1139656c8b0ed34d2867fbb834dad02bf0a",
        strip_prefix = "shellcheck-v{}".format(shellcheck_version),
    )
    http_archive(
        name = "shellcheck_darwin",
        urls = _github_public_urls("koalaman/shellcheck/releases/download/v{0}/shellcheck-v{0}.darwin.x86_64.tar.xz".format(shellcheck_version)),
        build_file = "@com_stripe_ruby_typer//third_party:shellcheck.BUILD",
        sha256 = "e065d4afb2620cc8c1d420a9b3e6243c84ff1a693c1ff0e38f279c8f31e86634",
        strip_prefix = "shellcheck-v{}".format(shellcheck_version),
    )

    # Needed to build CMake projects
    http_archive(
        name = "rules_foreign_cc",
        urls = _github_public_urls("bazelbuild/rules_foreign_cc/archive/d74623f0ad47f4e375de81baa454eb106715a416.zip"),
        sha256 = "47b61d25dd52bdaa1d571dab6705d076f05ba3d7a1bbbfed36145f8281c0403f",
        strip_prefix = "rules_foreign_cc-d74623f0ad47f4e375de81baa454eb106715a416",
    )

    register_ruby_dependencies()

def _github_public_urls(path):
    """
    Produce a url list that works both with github, and stripe's internal artifact cache.
    """
    return [
        "https://github.com/{}".format(path),
        "https://artifactory-content.stripe.build/artifactory/github-archives/{}".format(path),
    ]

def _emscripten_urls(path):
    """
    Produce a url list that works both with emscripten, and stripe's internal artifact cache.
    """
    return [
        "https://storage.googleapis.com/webassembly/emscripten-releases-builds/old/{}".format(path),
        "https://artifactory-content.stripe.build/artifactory/googleapis-storage-cache/webassembly/emscripten-releases-builds/old/{}".format(path),
    ]
