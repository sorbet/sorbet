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
    name="cxxopts",
    remote="https://github.com/jarro2783/cxxopts.git",
    commit="0b7686949d01f6475cc13ba0693725aefb76fc0c",
    build_file="//:cxxopts.BUILD",
  )
