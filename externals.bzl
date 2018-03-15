load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

# We define our externals here instead of directly in WORKSPACE
# because putting the `new_git_repository` calls here instead of there
# works around https://github.com/bazelbuild/bazel/issues/1465 when
# passing `build_file` to the `new_git_repository`.
def externals():
    native.new_http_archive(
        name = "gtest",
        url = "https://github.com/google/googletest/archive/release-1.8.0.zip",
        sha256 = "f3ed3b58511efd272eb074a3a6d6fb79d7c2e6a0e374323d1e6bcbcc1ef141bf",
        build_file = "gtest.BUILD",
        strip_prefix = "googletest-release-1.8.0",
    )

    native.new_http_archive(
            name = "yaml_cpp",
            url = "https://github.com/jbeder/yaml-cpp/archive/yaml-cpp-0.6.1.zip",
            sha256 = "2faaf686c140d9e690100a2e98ca0f4d325b0c13a67a26fd4b3b3a27fc476798",
            build_file = "//:yaml_cpp.BUILD",
            strip_prefix = "yaml-cpp-yaml-cpp-0.6.1",
        )

    # their zip archive has symlinks that bazel does not like
    new_git_repository(
        name="spdlog",
        remote="https://github.com/gabime/spdlog.git",
        commit="55680db160c3c486ccbbb40e10f6338e4d98e84d", # v0.16.3 - with eol customization
        build_file="//:spdlog.BUILD",
    )

    # proto_library, cc_proto_library, and java_proto_library rules implicitly
    # depend on @com_google_protobuf for protoc and proto runtimes.
    # This statement defines the @com_google_protobuf repo.
    native.http_archive(
        name = "com_google_protobuf",
        sha256 = "1f8b9b202e9a4e467ff0b0f25facb1642727cdf5e69092038f15b37c75b99e45",
        strip_prefix = "protobuf-3.5.1",
        urls = ["https://github.com/google/protobuf/archive/v3.5.1.zip"],
    )


    new_git_repository(
            name="lizard",
            remote="https://github.com/inikep/lizard.git",
            commit="6a1ed71450148c8aed57de3179b1bdd81800bada",
            build_file="//:lizard.BUILD",
        )

    new_git_repository(
            name="jemalloc",
            remote="https://github.com/jemalloc/jemalloc.git",
            commit="ae0f5d5c3f29beb9977148dedb58575757139586",
            build_file="//:jemalloc.BUILD",
        )

    new_git_repository(
            name="progressbar",
            remote="https://github.com/doches/progressbar.git",
            commit="c4c54f891ab05cfc411ec5c2ed147dd4cad1ccf3",
            build_file="//:progressbar.BUILD",
        )

    new_git_repository(
                name="concurrentqueue",
                remote="https://github.com/cameron314/concurrentqueue.git",
                commit="a5d7dd873c94b52973a8a2baab204034db0799bc",
                build_file="//:concurrentqueue.BUILD",
            )

    new_git_repository(
            name="statsd",
            remote="https://github.com/romanbsd/statsd-c-client",
            commit="0bfa3d59e29ad7eff332c9e2506a23d311ff8db4",
            build_file="//:statsd.BUILD",
        )

    new_git_repository(
        name="cxxopts",
        remote="https://github.com/jarro2783/cxxopts.git",
        commit="0b7686949d01f6475cc13ba0693725aefb76fc0c",
        build_file="//:cxxopts.BUILD",
    )

    native.new_http_archive(
        name="rang",
        url = "https://github.com/agauniyal/rang/archive/v3.1.0.zip",
        sha256="658adeb8a36d36981d4339fc839f2deedc0e75cb421db1982041d8a0a255835d",
        build_file = "rang.BUILD",
        strip_prefix = "rang-3.1.0",
    )

    git_repository(
            name="com_google_absl",
            remote="https://github.com/abseil/abseil-cpp.git",
            commit="055cc7dce10aa6bd7cc2ef64e0fe453fb792da62"
        )

    new_git_repository(
        name = "compdb",
        commit = "f9febfc7091e2585bafb1a96599ff517e169c3c4",
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

    native.new_http_archive(
        name="clang_6_0_0_darwin",
        url="http://releases.llvm.org/6.0.0/clang+llvm-6.0.0-x86_64-apple-darwin.tar.xz",
        build_file="//:clang.BUILD",
        sha256="0ef8e99e9c9b262a53ab8f2821e2391d041615dd3f3ff36fdf5370916b0f4268",
        type="tar.xz",
        strip_prefix="clang+llvm-6.0.0-x86_64-apple-darwin/",
    )

    native.new_http_archive(
        name="clang_6_0_0_linux",
        url="http://releases.llvm.org/6.0.0/clang+llvm-6.0.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz",
        build_file="//:clang.BUILD",
        sha256="114e78b2f6db61aaee314c572e07b0d635f653adc5d31bd1cd0bf31a3db4a6e5",
        type="tar.xz",
        strip_prefix="clang+llvm-6.0.0-x86_64-linux-gnu-ubuntu-14.04/",
    )
