#!/bin/bash

tmp="$(mktemp)"
infile="test/cli/suggest-sig-override/suggest-sig-override.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$(dirname "$tmp")" || exit 1
cp "$tmp" suggest-sig-override.rb

"$cwd/main/sorbet" -a suggest-sig-override.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat suggest-sig-override.rb

rm suggest-sig-override.rb
rm "$tmp"
