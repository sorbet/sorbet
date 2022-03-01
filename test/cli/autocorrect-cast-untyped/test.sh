#!/usr/bin/env bash

set -euo pipefail

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-cast-untyped/autocorrect-cast-untyped.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp"

if "$cwd/main/sorbet" --silence-dev-message -a autocorrect-cast-untyped.rb 2>&1; then
  echo "Expected failure, did not fail"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

cat autocorrect-cast-untyped.rb

rm autocorrect-cast-untyped.rb
rm "$tmp"
