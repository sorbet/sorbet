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

    new_git_repository(
        name="spdlog",
        remote="https://github.com/gabime/spdlog.git",
        commit="4fba14c79f356ae48d6141c561bf9fd7ba33fabd",
        build_file="//:spdlog.BUILD",
    )

    new_git_repository(
            name="progressbar",
            remote="https://github.com/doches/progressbar.git",
            commit="c4c54f891ab05cfc411ec5c2ed147dd4cad1ccf3",
            build_file="//:progressbar.BUILD",
        )

    new_git_repository(
            name="statsd",
            remote="https://github.com/romanbsd/statsd-c-client",
            commit="93c9df3831ac543a7ebed1684528d63de7575e05",
            build_file="//:statsd.BUILD",
        )

    new_git_repository(
        name="cxxopts",
        remote="https://github.com/jarro2783/cxxopts.git",
        commit="0b7686949d01f6475cc13ba0693725aefb76fc0c",
        build_file="//:cxxopts.BUILD",
    )

    git_repository(
            name="com_google_absl",
            remote="https://github.com/abseil/abseil-cpp.git",
            commit="778abb7c279547afe7a4642063712f13a80b8eea"
        )

    new_git_repository(
        name = "compdb",
        commit = "02c33ed2c0e86053073080fd215f44356ef5b543",
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
        name="clang_5_0_0_darwin",
        url="http://releases.llvm.org/5.0.0/clang+llvm-5.0.0-x86_64-apple-darwin.tar.xz",
        build_file="//:clang.BUILD",
        sha256="326be172ccb61210c9ae5dced27204977e249ec6589521cc30f82fd0904b0671",
        type="tar.xz",
        strip_prefix="clang+llvm-5.0.0-x86_64-apple-darwin/",
    )

    native.new_http_archive(
        name="clang_5_0_0_linux",
        url="https://releases.llvm.org/5.0.0/clang+llvm-5.0.0-linux-x86_64-ubuntu16.04.tar.xz",
        build_file="//:clang.BUILD",
        sha256="171968549a12d8cf1e308004a1c31450f663359731e1524b952665f80149284b",
        type="tar.xz",
        strip_prefix="clang+llvm-5.0.0-linux-x86_64-ubuntu16.04/",
    )
