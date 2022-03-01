#!/bin/bash

set -euo pipefail

if main/sorbet --silence-dev-message --lsp --disable-watchman test/cli/lsp-requires-input-dir/lsp-requires-input-dir.sh 2>&1; then
  echo "expected to fail, but it didn't!"
  exit 1
fi

echo --------------------------------------------------------------------------

tmpdir=$(mktemp -d)
# shellcheck disable=SC2064
trap "rm -rf $tmpdir" EXIT

old_pwd="$(pwd)"
cd "$tmpdir"

mkdir -p foo bar

if "$old_pwd/main/sorbet" --silence-dev-message --lsp --disable-watchman foo bar 2>&1; then
  echo "expected to fail, but it didn't!"
  exit 1
fi
