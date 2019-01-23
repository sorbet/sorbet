load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# We define our externals here instead of directly in WORKSPACE
# because putting the `new_git_repository` calls here instead of there
# works around https://github.com/bazelbuild/bazel/issues/1465 when
# passing `build_file` to the `new_git_repository`.
def externals():
    http_archive(
        name = "gtest",
        url = "https://github.com/google/googletest/archive/release-1.8.0.zip",
        sha256 = "f3ed3b58511efd272eb074a3a6d6fb79d7c2e6a0e374323d1e6bcbcc1ef141bf",
        build_file = "//third_party:gtest.BUILD",
        strip_prefix = "googletest-release-1.8.0",
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

    http_archive(
        name="cxxopts",
        url="https://github.com/jarro2783/cxxopts/archive/v2.1.2.zip",
        sha256="01897fd1930487f3e22879489721b5527ea5dbb32e40a57438e50d848cae22cf",
        build_file = "//third_party:cxxopts.BUILD",
        strip_prefix = "cxxopts-2.1.2",
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
        commit = "7bc80f9355b09466fffabce24d463d65e37fcc0f",
        remote = "https://github.com/grailbio/bazel-compilation-database.git",
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
        commit = "8a1359dc25add12a6e724f6a2bded60fbc23d08a",
        remote = "https://github.com/bazelbuild/buildifier.git",
    )


    # optimized version of blake2 hashing algorithm
    new_git_repository(
        name = "com_github_blake2_libb2",
        commit = "7feb2bb35dfe89750fba62bcd909409e995af754",
        remote ="https://github.com/BLAKE2/libb2",
        build_file = "//third_party:libb2.BUILD",
    )

    # portable reference implementation of blake2
    new_git_repository(
            name = "com_github_blake2_blake2",
            commit = "320c325437539ae91091ce62efec1913cd8093c2",
            remote ="https://github.com/BLAKE2/BLAKE2",
            build_file = "//third_party:blake2.BUILD",
        )

    new_git_repository(
        name = "com_github_msgpack_msgpack",
        commit = "daa78b46062d49bc192a921f7192637f58b334cc",
        remote = "https://github.com/msgpack/msgpack-c",
        build_file = "//third_party:msgpack.BUILD",
    )

    new_git_repository(
        name = "com_github_d_bahr_crcpp",
        commit = "76dc872e163ea91ca51468db686ba16912979765",
        remote = "https://github.com/d-bahr/CRCpp.git",
        build_file = "//third_party:crcpp.BUILD",
    )

    http_archive(
      name = "emscripten_toolchain",
      url = "https://github.com/kripken/emscripten/archive/1.38.21.tar.gz",
      build_file = "//third_party:emscripten-toolchain.BUILD",
      sha256 = "f3e6f1e6039256968131bc491ea8c2e1bfc31c41c1ec1370e2f1fae9a8e56faa",
      strip_prefix = "emscripten-1.38.21"
    )

    http_archive(
      name = "emscripten_clang_linux",
      url = "https://s3.amazonaws.com/mozilla-games/emscripten/packages/llvm/tag/linux_64bit/emscripten-llvm-e1.38.21.tar.gz",
      build_file = "//third_party:emscripten-clang.BUILD",
      sha256 = "355d64048529b77e076de84e5aa842773afbb672ccad1308f4cc734afe581a35",
      strip_prefix = "emscripten-llvm-e1.38.21",
    )

    http_archive(
      name = "emscripten_clang_darwin",
      url = "https://s3.amazonaws.com/mozilla-games/emscripten/packages/llvm/tag/osx_64bit/emscripten-llvm-e1.38.21.tar.gz",
      build_file = "//third_party:emscripten-clang.BUILD",
      sha256 = "ce473d2bf88dda0ec313c1899caaa586d69a4336cbc4bd2c6da1f5be217680cd",
      strip_prefix = "emscripten-llvm-e1.38.21",
    )

    git_repository(
        name="io_bazel_rules_ragel",
        remote="https://github.com/jmillikin/rules_ragel.git",
        commit="5723d752a53dd8e25eb4509f3ed869196a06cb2a"
    )
