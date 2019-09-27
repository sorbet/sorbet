load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# We define our externals here instead of directly in WORKSPACE
# because putting the `new_git_repository` calls here instead of there
# works around https://github.com/bazelbuild/bazel/issues/1465 when
# passing `build_file` to the `new_git_repository`.
def sorbet_llvm_externals():
    git_repository(
        name = "com_stripe_ruby_typer",
        remote = "https://github.com/sorbet/sorbet.git",
        commit = "50873fc9c1fa29cc6be4953630f4cf794651af49",
        shallow_since = "1568839565 -0700",
    )

    http_archive(
        name = "org_llvm_darwin",
        url = "https://releases.llvm.org/8.0.0/clang+llvm-8.0.0-x86_64-apple-darwin.tar.xz",
        build_file = "//third_party:llvm.BUILD",
        sha256 = "94ebeb70f17b6384e052c47fef24a6d70d3d949ab27b6c83d4ab7b298278ad6f",
        strip_prefix = "clang+llvm-8.0.0-x86_64-apple-darwin",
    )
