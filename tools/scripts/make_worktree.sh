#!/usr/bin/env bash

set -euo pipefail

if [ $# -eq 0 ]; then
  echo "usage: $0 <worktree_name>"
  exit 1
fi

worktree_name=$1
mkdir -p "$HOME/.cache/sorbet/_bazel_$worktree_name"
git worktree add "../$worktree_name" "HEAD@{0}"
cd "../$worktree_name"
echo "startup --output_base=$HOME/.cache/sorbet/_bazel_$worktree_name" > .bazelrc.local

echo "Created worktree at ../$worktree_name"
echo "Your first build in this worktree will be a little slow,"
echo "but other than that, we're done!"
