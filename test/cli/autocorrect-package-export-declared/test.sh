#!/usr/bin/env bash

root="$(pwd)"

cd "$(dirname "${BASH_SOURCE[0]}")" || exit 1

dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT

sources=({a..e})

# -RL avoids symlink problems in the bazel sandbox on systems with GNU cp
# (cp --recursive --dereference, but long options would fail on macOS)
cp -RL "${sources[@]}" "$dir"

"$root/main/sorbet" \
  --censor-for-snapshot-tests --silence-dev-message --max-threads=0 \
  --sorbet-packages -a "$dir" 2>&1

"$root/main/sorbet" \
  --censor-for-snapshot-tests --silence-dev-message --max-threads=0 \
  --sorbet-packages "$dir" 2>&1
