load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# We define our externals here instead of directly in WORKSPACE
# because putting the `new_git_repository` calls here instead of there
# works around https://github.com/bazelbuild/bazel/issues/1465 when
# passing `build_file` to the `new_git_repository`.
def externals():
    git_repository(
        name="com_google_googletest",
        remote="https://github.com/google/googletest.git",
        commit="9518a57428ae0a7ed450c1361768e84a2a38af5a"
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
        name="spdlog",
        remote="https://github.com/gabime/spdlog.git",
        commit="a7148b718ea2fabb8387cb90aee9bf448da63e65", # v1.3.1
        build_file = "//third_party:spdlog.BUILD",
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
        name="libprotobuf-mutator",
        remote="https://github.com/google/libprotobuf-mutator.git",
        commit="3d1ea5f9eb5fc90f9f8e28447541929482cfb049",
        build_file = "//third_party:libprotobuf-mutator.BUILD",
    )

    new_git_repository(
        name="lmdb",
        remote="https://github.com/DarkDimius/lmdb.git",
        commit="15a9c2604e3401593110ddf6c9e2e16a4b28e68e",
        build_file = "//third_party:lmdb.BUILD",
    )

    new_git_repository(
        name="rapidjson",
        remote="https://github.com/Tencent/rapidjson.git",
        commit="bfdcf4911047688fec49014d575433e2e5eb05be",
        build_file = "//third_party:rapidjson.BUILD",
    )

    new_git_repository(
        name="lizard",
        remote="https://github.com/inikep/lizard.git",
        commit="02491c71c2e6fd5c10997404df2f18d0fc7afadb",
        build_file = "//third_party:lizard.BUILD",
    )

    new_git_repository(
        name="pdqsort",
        remote="https://github.com/orlp/pdqsort.git",
        commit="08879029ab8dcb80a70142acb709e3df02de5d37",
        build_file = "//third_party:pdqsort.BUILD",
    )

    new_git_repository(
        name="jemalloc",
        remote="https://github.com/jemalloc/jemalloc.git",
        commit="0ff7ff3ec7b322881fff3bd6d4861fda6e9331d9", # 5.1.0 with some tunning patches
        build_file = "//third_party:jemalloc.BUILD",
    )

    native.new_local_repository(
        name="progressbar",
        path="third_party/progressbar",
        build_file="//third_party:progressbar.BUILD",
     )

    new_git_repository(
        name="concurrentqueue",
        remote="https://github.com/cameron314/concurrentqueue.git",
        commit="8f65a8734d77c3cc00d74c0532efca872931d3ce",
        build_file = "//third_party:concurrentqueue.BUILD",
    )

    new_git_repository(
        name="statsd",
        remote="https://github.com/romanbsd/statsd-c-client.git",
        commit="0caa5ef05d6a786bb4695394534a7182a3c94427",
        build_file = "//third_party:statsd.BUILD",
    )

    new_git_repository(
        name="cxxopts",
        remote="https://github.com/jarro2783/cxxopts.git",
        commit="9990f73845d76106063536d7cd630ac15cb4a065",
        build_file = "//third_party:cxxopts.BUILD",
    )

    http_archive(
        name="rang",
        url = "https://github.com/agauniyal/rang/archive/v3.1.0.zip",
        sha256="658adeb8a36d36981d4339fc839f2deedc0e75cb421db1982041d8a0a255835d",
        build_file = "//third_party:rang.BUILD",
        strip_prefix = "rang-3.1.0",
    )

    git_repository(
        name="com_google_absl",
        remote="https://github.com/abseil/abseil-cpp.git",
        commit="0b1e6d417b414aad9282e32e8c49c719edeb63c1"
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
    )

    native.new_local_repository(
        name="parser",
        path="third_party/parser",
        build_file="//third_party:parser.BUILD",
    )

    git_repository(
        name = "com_grail_bazel_toolchain",
        remote = "https://github.com/DarkDimius/bazel-toolchain.git",
        commit="cbbedc03065f9d6806395cf4b95b4bcb4ed37025",
    )

    git_repository(
        name = "io_bazel_rules_go",
        remote = "https://github.com/bazelbuild/rules_go.git",
        commit = "153c823428660f14b6e028cc71086833e445b2da",
    )

    git_repository(
        name = "com_github_bazelbuild_buildtools",
        remote = "https://github.com/bazelbuild/buildifier.git",
        commit = "8a1359dc25add12a6e724f6a2bded60fbc23d08a",
    )


    # optimized version of blake2 hashing algorithm
    new_git_repository(
        name = "com_github_blake2_libb2",
        remote ="https://github.com/BLAKE2/libb2",
        commit = "7feb2bb35dfe89750fba62bcd909409e995af754",
        build_file = "//third_party:libb2.BUILD",
    )

    # portable reference implementation of blake2
    new_git_repository(
        name = "com_github_blake2_blake2",
        remote ="https://github.com/BLAKE2/BLAKE2",
        commit = "320c325437539ae91091ce62efec1913cd8093c2",
        build_file = "//third_party:blake2.BUILD",
    )

    new_git_repository(
        name = "com_github_msgpack_msgpack",
        remote = "https://github.com/msgpack/msgpack-c",
        commit = "daa78b46062d49bc192a921f7192637f58b334cc",
        build_file = "//third_party:msgpack.BUILD",
    )

    new_git_repository(
        name = "com_github_d_bahr_crcpp",
        remote = "https://github.com/d-bahr/CRCpp.git",
        commit = "76dc872e163ea91ca51468db686ba16912979765",
        build_file = "//third_party:crcpp.BUILD",
    )

    http_archive(
        name = "emscripten_toolchain",
        url = "https://github.com/kripken/emscripten/archive/1.38.25.tar.gz",
        build_file = "//third_party:emscripten-toolchain.BUILD",
        sha256 = "4d6fa350895fabc25b89ce5f9dcb528e719e7c2bf7dacab2a3e3cc818ecd7019",
        strip_prefix = "emscripten-1.38.25"
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
        name="io_bazel_rules_ragel",
        remote="https://github.com/jmillikin/rules_ragel.git",
        commit="5723d752a53dd8e25eb4509f3ed869196a06cb2a"
    )

    git_repository(
        name="io_bazel_rules_bison",
        remote="https://github.com/jmillikin/rules_bison.git",
        commit="3809da0cea172c320f1fb7cd94bcb9be97897b14"
    )

    git_repository(
        name = "io_bazel_rules_m4",
        remote = "https://github.com/jmillikin/rules_m4",
        commit = "ae19f8df57f680c6dbad4887e162ca17ee97a13e",
    )
