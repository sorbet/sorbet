#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/suggest-type-alias/suggest-type-alias.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a suggest-type-alias.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat suggest-type-alias.rb

rm suggest-type-alias.rb
rm "$tmp"
