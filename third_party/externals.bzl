load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

# We define our externals here instead of directly in WORKSPACE
def register_sorbet_dependencies():
    # At some point the builtin @platforms package will be removed, and we'll no longer be able to refer to
    # @platforms//os:macos etc. The long-term workaround for this is to depend directly on bazelbuild/platforms as
    # @platforms. See https://github.com/bazelbuild/bazel/issues/8622 for more information.
    http_archive(
        name = "platforms",
        urls = _github_public_urls("bazelbuild/platforms/archive/d4c9d7f51a7c403814b60f66d20eeb425fbaaacb.zip"),
        sha256 = "a5058ac93023092c406432ec650f30ec5a8c75d8b9d13c73150f60a9050a5663",
        strip_prefix = "platforms-d4c9d7f51a7c403814b60f66d20eeb425fbaaacb",
    )

    http_archive(
        name = "doctest",
        urls = _github_public_urls("onqtam/doctest/archive/7d42bd0fab6c44010c8aed9338bd02bea5feba41.zip"),
        sha256 = "b33c8e954d15a146bb744ca29f4ca204b955530f52b2f8a895746a99cee4f2df",
        build_file = "@com_stripe_ruby_typer//third_party:doctest.BUILD",
        strip_prefix = "doctest-7d42bd0fab6c44010c8aed9338bd02bea5feba41",
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
        urls = _github_public_urls("jarro2783/cxxopts/archive/b0f67a06de3446aa97a4943ad0ad6086460b2b61.zip"),
        sha256 = "8635d7305e6623e7f4c635dae901891eb1151cee3106445d124c696361bb70fc",
        build_file = "@com_stripe_ruby_typer//third_party:cxxopts.BUILD",
        strip_prefix = "cxxopts-b0f67a06de3446aa97a4943ad0ad6086460b2b61",
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

    # NOTE: we use the sorbet branch for development to keep our changes rebasable on grailio/bazel-toolchain
    http_archive(
        name = "com_grail_bazel_toolchain",
        urls = _github_public_urls("sorbet/bazel-toolchain/archive/a685e1e6bd1e7cc9a5b84f832539585bb68d8ab4.zip"),
        sha256 = "90c59f14cada755706a38bdd0f5ad8f0402cbf766387929cfbee9c3f1b4c82d7",
        strip_prefix = "bazel-toolchain-a685e1e6bd1e7cc9a5b84f832539585bb68d8ab4",
    )

    http_archive(
        name = "io_bazel_rules_go",
        urls = _github_public_urls("bazelbuild/rules_go/archive/dd4fb4f8128b83f189f7bdda663e65b915a6d3c4.zip"),
        sha256 = "ea702009018b6a5d6665808a4d1f54e2f40a2938e3946e98de00d38b34fd8a27",
        strip_prefix = "rules_go-dd4fb4f8128b83f189f7bdda663e65b915a6d3c4",
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
        urls = _github_public_urls("jmillikin/rules_m4/releases/download/v0.2/rules_m4-v0.2.tar.xz"),
        sha256 = "c67fa9891bb19e9e6c1050003ba648d35383b8cb3c9572f397ad24040fb7f0eb",
    )

    http_archive(
        name = "cpp_subprocess",
        urls = _github_public_urls("arun11299/cpp-subprocess/archive/9c624ce4e3423cce9f148bafbae56abfd6437ea0.zip"),
        sha256 = "1810d1ec80f3c319dcbb530443b264b9a32a449b5a5d3630076e473648bba8cc",
        build_file = "@com_stripe_ruby_typer//third_party:cpp_subprocess.BUILD",
        strip_prefix = "cpp-subprocess-9c624ce4e3423cce9f148bafbae56abfd6437ea0",
    )

    native.new_local_repository(
        name = "system_ssl_darwin",
        path = "/usr/local/opt/openssl",
        build_file = "@com_stripe_ruby_typer//third_party/openssl:darwin.BUILD",
    )

    native.new_local_repository(
        name = "system_ssl_linux",
        path = "/usr",
        build_file = "@com_stripe_ruby_typer//third_party/openssl:linux.BUILD",
    )

    http_archive(
        name = "rules_rust",
        sha256 = "727b93eb5d57ec411f2afda7e3993e22d7772d0b2555ba745c3dec7323ea955a",
        strip_prefix = "rules_rust-0768a7f00de134910c3cbdab7bbfdd011d995766",

        # Master branch as of 2021-06-29
        urls = _github_public_urls("bazelbuild/rules_rust/archive/0768a7f00de134910c3cbdab7bbfdd011d995766.tar.gz"),
    )

    http_archive(
        name = "bazel_skylib",
        sha256 = "9a737999532daca978a158f94e77e9af6a6a169709c0cee274f0a4c3359519bd",
        strip_prefix = "bazel-skylib-1.0.0",
        urls = _github_public_urls("bazelbuild/bazel-skylib/archive/1.0.0.tar.gz"),
    )

    http_archive(
        name = "llvm",

        # llvm 12.0.0
        urls = _github_public_urls("llvm/llvm-project/archive/0cbbf06b625605fff83d89b17c2187c7ccfcecd5.tar.gz"),
        build_file = "@com_stripe_ruby_typer//third_party/llvm:llvm.autogenerated.BUILD",
        sha256 = "cd4964439e7b4a2a22176ec2de70c3b67771c515eacaf88fb82a3a52fed7592a",
        strip_prefix = "llvm-project-0cbbf06b625605fff83d89b17c2187c7ccfcecd5/llvm",
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
        urls = _ruby_urls("3.0/ruby-3.0.3.tar.gz"),
        sha256 = "3586861cb2df56970287f0fd83f274bd92058872d830d15570b36def7f1a92ac",
        strip_prefix = "ruby-3.0.3",
        build_file = "@com_stripe_ruby_typer//third_party/ruby:ruby.BUILD",
    )

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
