#!/usr/bin/env bash

set -euo pipefail

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-shape-square-brackets-eq/autocorrect-shape-square-brackets-eq.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

if "$cwd/main/sorbet" --silence-dev-message -a autocorrect-shape-square-brackets-eq.rb 2>&1; then
  echo "Expected to fail, but didn't"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat autocorrect-shape-square-brackets-eq.rb

rm autocorrect-shape-square-brackets-eq.rb
rm "$tmp"
