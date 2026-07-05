#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/suggest-sig/suggest-sig.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --censor-for-snapshot-tests --silence-dev-message -a suggest-sig.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat suggest-sig.rb

rm suggest-sig.rb
rm "$tmp"
