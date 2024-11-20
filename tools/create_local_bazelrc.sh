#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "$0")/.."

# The .bazelrc.local will live in the sorbet repo so it doesn't interfere with
# other bazel-based repos you have.
echo "build  --disk_cache=$HOME/.cache/sorbet/bazel-cache" >> ".bazelrc.local"
echo "test   --disk_cache=$HOME/.cache/sorbet/bazel-cache" >> ".bazelrc.local"
mkdir -p "$HOME/.cache/sorbet/bazel-cache"
