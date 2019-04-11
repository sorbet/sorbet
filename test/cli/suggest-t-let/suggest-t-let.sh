#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/suggest-t-let/suggest-t-let.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a suggest-t-let.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat suggest-t-let.rb

rm suggest-t-let.rb
rm "$tmp"
