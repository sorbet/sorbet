#!/bin/bash

tmp="$(mktemp)"
infile="test/cli/suggest-sig-literal/suggest-sig-literal.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$(dirname "$tmp")" || exit 1
cp "$tmp" suggest-sig-literal.rb

"$cwd/main/sorbet" --silence-dev-message -a suggest-sig-literal.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat suggest-sig-literal.rb

rm suggest-sig-literal.rb
rm "$tmp"
