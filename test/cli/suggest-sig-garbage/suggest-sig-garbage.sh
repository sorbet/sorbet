#!/bin/bash

tmp="$(mktemp)"
infile="test/cli/suggest-sig-garbage/suggest-sig-garbage.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$(dirname "$tmp")" || exit 1
cp "$tmp" suggest-sig-garbage.rb

"$cwd/main/sorbet" -a --suggest-runtime-profiled suggest-sig-garbage.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat suggest-sig-garbage.rb

rm suggest-sig-garbage.rb
rm "$tmp"
