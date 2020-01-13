#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/correct-bare-stdlib-generics/correct-bare-stdlib-generics.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a correct-bare-stdlib-generics.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat correct-bare-stdlib-generics.rb

rm correct-bare-stdlib-generics.rb
rm -r "$tmp"
