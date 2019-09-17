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
        commit = "7b47c31a01b86cd67da1eaa223c31c08192335ad",
    )
    
    # add new stuff here
