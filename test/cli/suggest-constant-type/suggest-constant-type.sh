#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/suggest-constant-type/suggest-constant-type.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a suggest-constant-type.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat suggest-constant-type.rb

rm suggest-constant-type.rb
rm "$tmp"
