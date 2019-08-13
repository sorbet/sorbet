load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# We define our externals here instead of directly in WORKSPACE
# because putting the `new_git_repository` calls here instead of there
# works around https://github.com/bazelbuild/bazel/issues/1465 when
# passing `build_file` to the `new_git_repository`.
def externals():
    git_repository(
        name = "com_google_googletest",
        remote = "https://github.com/google/googletest.git",
        commit = "9518a57428ae0a7ed450c1361768e84a2a38af5a",
        shallow_since = "1547838363 -0500",
    )

    http_archive(
        name = "yaml_cpp",
        url = "https://github.com/jbeder/yaml-cpp/archive/yaml-cpp-0.6.2.zip",
        sha256 = "292c8de66bfda19a2ca08a32a8c1ec39b709ac75f54e6be0735940db2dbdff76",
        build_file = "//third_party:yaml_cpp.BUILD",
        strip_prefix = "yaml-cpp-yaml-cpp-0.6.2",
    )

    # their zip archive has symlinks that bazel does not like
    new_git_repository(
        name = "spdlog",
        remote = "https://github.com/gabime/spdlog.git",
        commit = "a7148b718ea2fabb8387cb90aee9bf448da63e65",  # v1.3.1
        build_file = "//third_party:spdlog.BUILD",
        shallow_since = "1547806387 +0200",
    )

    # proto_library, cc_proto_library, and java_proto_library rules implicitly
    # depend on @com_google_protobuf for protoc and proto runtimes.
    # This statement defines the @com_google_protobuf repo.
    http_archive(
        name = "com_google_protobuf",
        sha256 = "d7a221b3d4fb4f05b7473795ccea9e05dab3b8721f6286a95fffbffc2d926f8b",
        strip_prefix = "protobuf-3.6.1",
        urls = ["https://github.com/google/protobuf/archive/v3.6.1.zip"],
    )

    new_git_repository(
        name = "libprotobuf-mutator",
        remote = "https://github.com/google/libprotobuf-mutator.git",
        commit = "3d1ea5f9eb5fc90f9f8e28447541929482cfb049",
        build_file = "//third_party:libprotobuf-mutator.BUILD",
        shallow_since = "1549313935 -0800",
    )

    new_git_repository(
        name = "lmdb",
        remote = "https://github.com/DarkDimius/lmdb.git",
        commit = "15a9c2604e3401593110ddf6c9e2e16a4b28e68e",
        build_file = "//third_party:lmdb.BUILD",
        shallow_since = "1539245726 -0700",
    )

    new_git_repository(
        name = "rapidjson",
        remote = "https://github.com/Tencent/rapidjson.git",
        commit = "01950eb7acec78818d68b762efc869bba2420d82",
        build_file = "//third_party:rapidjson.BUILD",
        shallow_since = "1555291518 +0800",
    )

    new_git_repository(
        name = "lizard",
        remote = "https://github.com/inikep/lizard.git",
        commit = "dda3b335e92ecd5caceccc9c577b39dd4e3c9950",
        build_file = "//third_party:lizard.BUILD",
        shallow_since = "1552038096 +0100",
    )

    new_git_repository(
        name = "pdqsort",
        remote = "https://github.com/orlp/pdqsort.git",
        commit = "08879029ab8dcb80a70142acb709e3df02de5d37",
        build_file = "//third_party:pdqsort.BUILD",
        shallow_since = "1524162080 +0200",
    )

    new_git_repository(
        name = "jemalloc",
        remote = "https://github.com/jemalloc/jemalloc.git",
        commit = "b0b3e49a54ec29e32636f4577d9d5a896d67fd20",  # 5.2.0
        build_file = "//third_party:jemalloc.BUILD",
        shallow_since = "1554252642 -0700",
    )

    native.new_local_repository(
        name = "progressbar",
        path = "third_party/progressbar",
        build_file = "//third_party:progressbar.BUILD",
    )

    new_git_repository(
        name = "concurrentqueue",
        remote = "https://github.com/cameron314/concurrentqueue.git",
        commit = "dea078cf5b6e742cd67a0d725e36f872feca4de4",
        build_file = "//third_party:concurrentqueue.BUILD",
        shallow_since = "1547235167 -0500",
    )

    new_git_repository(
        name = "statsd",
        remote = "https://github.com/romanbsd/statsd-c-client.git",
        commit = "0caa5ef05d6a786bb4695394534a7182a3c94427",
        build_file = "//third_party:statsd.BUILD",
        shallow_since = "1520851215 +0200",
    )

    new_git_repository(
        name = "cxxopts",
        remote = "https://github.com/jarro2783/cxxopts.git",
        commit = "e34676f73e49eeff30cb101f1c5ba8806fbe6773",
        build_file = "//third_party:cxxopts.BUILD",
        shallow_since = "1557700395 +1000",
    )

    http_archive(
        name = "rang",
        url = "https://github.com/agauniyal/rang/archive/v3.1.0.zip",
        sha256 = "658adeb8a36d36981d4339fc839f2deedc0e75cb421db1982041d8a0a255835d",
        build_file = "//third_party:rang.BUILD",
        strip_prefix = "rang-3.1.0",
    )

    git_repository(
        name = "com_google_absl",
        remote = "https://github.com/abseil/abseil-cpp.git",
        commit = "d65e19dfcd8697076f68598c0131c6930cdcd74d",
        shallow_since = "1561428275 -0400",
    )

    new_git_repository(
        name = "compdb",
        remote = "https://github.com/grailbio/bazel-compilation-database.git",
        commit = "7bc80f9355b09466fffabce24d463d65e37fcc0f",
        build_file_content = (
            """
package(default_visibility = ["//visibility:public"])
"""
        ),
        shallow_since = "1541620420 -0800",
    )

    native.new_local_repository(
        name = "parser",
        path = "third_party/parser",
        build_file = "//third_party:parser.BUILD",
    )

    # NOTE: using this branch:
    # https://github.com/DarkDimius/bazel-toolchain/tree/dp-srb-now
    git_repository(
        name = "com_grail_bazel_toolchain",
        remote = "https://github.com/DarkDimius/bazel-toolchain.git",
        commit = "fc345f29d7a2b5dd431be006b36be1b3d3936987",
        shallow_since = "1561409079 -0700",
    )

    git_repository(
        name = "io_bazel_rules_go",
        remote = "https://github.com/bazelbuild/rules_go.git",
        commit = "153c823428660f14b6e028cc71086833e445b2da",
        shallow_since = "1545156968 -0500",
    )

    git_repository(
        name = "com_github_bazelbuild_buildtools",
        remote = "https://github.com/bazelbuild/buildifier.git",
        commit = "8a1359dc25add12a6e724f6a2bded60fbc23d08a",
        shallow_since = "1537534502 +0200",
    )

    # optimized version of blake2 hashing algorithm
    new_git_repository(
        name = "com_github_blake2_libb2",
        remote = "https://github.com/BLAKE2/libb2",
        commit = "7feb2bb35dfe89750fba62bcd909409e995af754",
        build_file = "//third_party:libb2.BUILD",
        shallow_since = "1531310625 +0100",
    )

    # portable reference implementation of blake2
    new_git_repository(
        name = "com_github_blake2_blake2",
        remote = "https://github.com/BLAKE2/BLAKE2",
        commit = "320c325437539ae91091ce62efec1913cd8093c2",
        build_file = "//third_party:blake2.BUILD",
        shallow_since = "1531310717 +0100",
    )

    new_git_repository(
        name = "com_github_msgpack_msgpack",
        remote = "https://github.com/msgpack/msgpack-c",
        commit = "9389912eaf4358e2ad1621b3995b5b26cd6703f3",
        build_file = "//third_party:msgpack.BUILD",
        shallow_since = "1557663135 +0900",
    )

    new_git_repository(
        name = "com_github_d_bahr_crcpp",
        remote = "https://github.com/d-bahr/CRCpp.git",
        commit = "534c1d8c5517cfbb0a0f1ff0d9ec4c8b8ffd78e2",
        build_file = "//third_party:crcpp.BUILD",
        shallow_since = "1557307551 -0700",
    )

    http_archive(
        name = "emscripten_toolchain",
        url = "https://github.com/kripken/emscripten/archive/1.38.25.tar.gz",
        build_file = "//third_party:emscripten-toolchain.BUILD",
        sha256 = "4d6fa350895fabc25b89ce5f9dcb528e719e7c2bf7dacab2a3e3cc818ecd7019",
        strip_prefix = "emscripten-1.38.25",
    )

    http_archive(
        name = "emscripten_clang_linux",
        url = "https://s3.amazonaws.com/mozilla-games/emscripten/packages/llvm/tag/linux_64bit/emscripten-llvm-e1.38.25.tar.gz",
        build_file = "//third_party:emscripten-clang.BUILD",
        sha256 = "0e9a5a114a60c21604f4038b573109bd31424aeba275b4173480485ca0a56ba4",
        strip_prefix = "emscripten-llvm-e1.38.25",
    )

    http_archive(
        name = "emscripten_clang_darwin",
        url = "https://s3.amazonaws.com/mozilla-games/emscripten/packages/llvm/tag/osx_64bit/emscripten-llvm-e1.38.25.tar.gz",
        build_file = "//third_party:emscripten-clang.BUILD",
        sha256 = "01519125c613d0b013193eaf5ac5031e6ec34aac2451c357fd4097874ceee38c",
        strip_prefix = "emscripten-llvm-e1.38.25",
    )

    git_repository(
        name = "io_bazel_rules_ragel",
        remote = "https://github.com/jmillikin/rules_ragel.git",
        commit = "5723d752a53dd8e25eb4509f3ed869196a06cb2a",
        shallow_since = "1547960305 -0800",
    )

    git_repository(
        name = "io_bazel_rules_bison",
        remote = "https://github.com/jmillikin/rules_bison.git",
        commit = "3809da0cea172c320f1fb7cd94bcb9be97897b14",
        shallow_since = "1549513081 -0800",
    )

    git_repository(
        name = "io_bazel_rules_m4",
        remote = "https://github.com/jmillikin/rules_m4",
        commit = "ae19f8df57f680c6dbad4887e162ca17ee97a13e",
        shallow_since = "1549512390 -0800",
    )

    new_git_repository(
        name = "cpp_subprocess",
        remote = "https://github.com/arun11299/cpp-subprocess.git",
        commit = "de5f791d0457ffa866c371f16a3a53228515bb9a",
        build_file = "//third_party:cpp_subprocess.BUILD",
        shallow_since = "1552288452 +0530",
    )

    http_archive(
        name = "ruby_2_4_3",
        url = "https://cache.ruby-lang.org/pub/ruby/2.4/ruby-2.4.3.tar.gz",
        sha256 = "fd0375582c92045aa7d31854e724471fb469e11a4b08ff334d39052ccaaa3a98",
        strip_prefix = "ruby-2.4.3",
        build_file = "//third_party/ruby:ruby-2.4.BUILD",
        patches = [
            "//third_party/ruby:probes.h.patch",
            "//third_party/ruby:enc.encinit.c.patch",
        ],
        patch_args = ["-p1"],
    )

    http_archive(
        name = "ruby_2_6_3",
        url = "https://cache.ruby-lang.org/pub/ruby/2.6/ruby-2.6.3.tar.gz",
        sha256 = "577fd3795f22b8d91c1d4e6733637b0394d4082db659fccf224c774a2b1c82fb",
        strip_prefix = "ruby-2.6.3",
        build_file = "//third_party/ruby:ruby-2.6.BUILD",
        patches = [
            "//third_party/ruby:probes.h.patch",
            "//third_party/ruby:enc.encinit.c.patch",
            "//third_party/ruby:debug_counter.h.patch",
        ],
        patch_args = ["-p1"],
    )

    http_archive(
        name = "zlib",
        url = "https://zlib.net/zlib-1.2.11.tar.gz",
        sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
        strip_prefix = "zlib-1.2.11",
        build_file = "//third_party:zlib.BUILD",
    )
