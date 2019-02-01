#!/bin/bash

tmp="$(mktemp)"
infile="test/cli/suggest-type-alias/suggest-type-alias.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$(dirname "$tmp")" || exit 1
cp "$tmp" suggest-type-alias.rb

"$cwd/main/sorbet" --silence-dev-message -a suggest-type-alias.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat suggest-type-alias.rb

rm suggest-type-alias.rb
rm "$tmp"
