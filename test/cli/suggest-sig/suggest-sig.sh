#!/bin/bash

tmp="$(mktemp)"
infile="test/cli/suggest-sig/suggest-sig.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$(dirname "$tmp")" || exit 1
cp "$tmp" suggest-sig.rb

"$cwd/main/sorbet" --silence-dev-message -a suggest-sig.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat suggest-sig.rb

rm suggest-sig.rb
rm "$tmp"
