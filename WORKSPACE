workspace(name = "com_stripe_sruby")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

BAZEL_VERSION = "0.5.4"
BAZEL_VERSION_SHA = "2157b05309614d6af0e4bbc6065987aede590822634a0522161f3af5d647abc9"

new_http_archive(
    name = "gtest",
    url = "https://github.com/google/googletest/archive/release-1.8.0.zip",
    sha256 = "f3ed3b58511efd272eb074a3a6d6fb79d7c2e6a0e374323d1e6bcbcc1ef141bf",
    build_file = "gtest.BUILD",
    strip_prefix = "googletest-release-1.8.0",
)

#new_http_archive(
#    name = "spdlog",
#    url = "https://github.com/gabime/spdlog/archive/v0.14.0.zip",
#    sha256 = "6ddc50dbb16b41300d16f8a82ef2cc94ce6b0c72d7441e6db114a6dc7dfee5b6",
#    build_file = "spdlog.BUILD",
#    strip_prefix = "googletest-release-1.8.",
#)

new_git_repository(
   name="spdlog",
   remote="https://github.com/gabime/spdlog.git",
   commit="4fba14c79f356ae48d6141c561bf9fd7ba33fabd",
   build_file_content="""
cc_library(
    name = "spdlog",
    srcs = [],
    hdrs = [
        "include/spdlog/spdlog.h",
        "include/spdlog/tweakme.h",
    ],
    copts = [
        "-Iexternal/spdlog/",
    ],
    includes = [
        "include/",
    ],
    visibility = ["//visibility:public"],
    linkopts = ["-pthread"],
)
"""
)
