#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-block-pass-curly-braces/autocorrect-block-pass-curly-braces.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a autocorrect-block-pass-curly-braces.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat autocorrect-block-pass-curly-braces.rb

rm autocorrect-block-pass-curly-braces.rb
rm "$tmp"
