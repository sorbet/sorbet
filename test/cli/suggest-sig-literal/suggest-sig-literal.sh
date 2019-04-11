#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/suggest-sig-literal/suggest-sig-literal.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a suggest-sig-literal.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat suggest-sig-literal.rb

rm suggest-sig-literal.rb
rm "$tmp"
