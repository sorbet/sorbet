#!/bin/bash

tmp="$(mktemp -d)"
cp "test/cli/autocorrect-requires-ancestor-block/autocorrect-requires-ancestor-block.rb" "$tmp"

old_pwd="$(pwd)"
cd "$tmp" || exit 1

"$old_pwd/main/sorbet" --censor-for-snapshot-tests --silence-dev-message --enable-experimental-requires-ancestor -a autocorrect-requires-ancestor-block.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat autocorrect-requires-ancestor-block.rb
rm -rf "$tmp"
