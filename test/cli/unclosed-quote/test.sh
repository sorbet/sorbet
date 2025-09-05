#!/usr/bin/env bash

set -euo pipefail

tmp="$(mktemp -d)"
cp test/cli/unclosed-quote/*.rb "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

# Using bash range here because sometimes the glob order sorting is OS/shell dependent.
for file in ./unclosed_quote_{1..7}.rb; do
  echo
  echo ----- "$file" ---------------------------------------
  echo

  if "$cwd/main/sorbet" --silence-dev-message --print=desugar-tree -a "$file" 2>&1; then
    echo "Expected to fail, but didn't"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make sure that 'extend' is only added once per class.
  cat "$file"
done

cd - &> /dev/null
rm -rf "$tmp"
