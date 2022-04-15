#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-useless-cast/autocorrect-useless-cast.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a autocorrect-useless-cast.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat autocorrect-useless-cast.rb

rm autocorrect-useless-cast.rb
rm "$tmp"
