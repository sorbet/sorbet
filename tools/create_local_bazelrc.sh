#!/usr/bin/env bash

set -euo pipefail

# Usage: tools/create_local_bazelrc.sh [--share-llvm | --unshare-llvm]
# For --share-llvm, see "Multiple checkouts and worktrees" in README.md.

cd "$(dirname "$0")/.."

rc=".bazelrc.local"
touch "$rc"

add_line() {
  if ! grep -qxF -- "$1" "$rc"; then
    echo "$1" >> "$rc"
  fi
}

# The .bazelrc.local will live in the sorbet repo so it doesn't interfere with
# other bazel-based repos you have.
add_line "build  --disk_cache=$HOME/.cache/sorbet/bazel-cache"
add_line "test   --disk_cache=$HOME/.cache/sorbet/bazel-cache"
mkdir -p "$HOME/.cache/sorbet/bazel-cache"

llvm_dir="$HOME/.cache/sorbet/llvm"

case "${1:-}" in
  "")
    exit 0
    ;;
  --share-llvm | --unshare-llvm)
    # Drop any earlier override, so that Bazel sees WORKSPACE's own definition.
    grep -vF -- "=$llvm_dir/" "$rc" > "$rc.tmp" || true
    mv "$rc.tmp" "$rc"
    ;;
  *)
    echo "usage: $0 [--share-llvm | --unshare-llvm]" >&2
    exit 1
    ;;
esac

if [ "$1" = "--share-llvm" ]; then
  # toolchains_llvm's `llvm` rule downloads and extracts the distribution. Its
  # name includes the LLVM version.
  for repo in $(./bazel query 'kind("^llvm rule$", //external:*)' 2> /dev/null | sed 's|^//external:||'); do
    if [ ! -d "$llvm_dir/$repo" ]; then
      ./bazel fetch "@$repo//:all"
      mkdir -p "$llvm_dir"
      mv "$(./bazel info output_base)/external/$repo" "$llvm_dir/$repo"
    fi
    echo "common --override_repository=$repo=$llvm_dir/$repo" >> "$rc"
  done
fi
