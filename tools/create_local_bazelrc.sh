#!/usr/bin/env bash

set -euo pipefail

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
