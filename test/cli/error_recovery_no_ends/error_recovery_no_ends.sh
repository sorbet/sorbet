#!/usr/bin/env bash

set -euo pipefail

stderr_log="$(mktemp)"
trap 'rm -rf "$stderr_log"' EXIT

if main/sorbet \
    --silence-dev-message \
    --print=index-tree \
    test/cli/error_recovery_no_ends/error_recovery_no_ends.rb 2> "$stderr_log"; then
  echo "Expected to fail!"
  exit 1
fi

echo -------------------------------------

cat "$stderr_log"
