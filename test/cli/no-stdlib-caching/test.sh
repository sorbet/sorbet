#!/usr/bin/env bash

set -euo pipefail

dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT

cp ./test/cli/no-stdlib-caching/test.rb "$dir"

main/sorbet --silence-dev-message --cache-dir="$dir/cache" "$dir" 2>&1 || true
main/sorbet --silence-dev-message --no-stdlib --cache-dir="$dir/cache" "$dir" 2>&1 || true

