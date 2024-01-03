#!/usr/bin/env bash

set -euo pipefail

args=(
  --censor-for-snapshot-tests
  --silence-dev-message
  --print=file-table-json
  --max-threads=0
  test/cli/track-untyped
)

echo --- implicit 'everywhere' ---
main/sorbet "${args[@]}" --track-untyped 2>&1 || true

echo --- explicit 'nowhere' ---
main/sorbet "${args[@]}" --track-untyped=nowhere 2>&1 || true

echo --- explicit 'everywhere' ---
main/sorbet "${args[@]}" --track-untyped=everywhere 2>&1 || true
