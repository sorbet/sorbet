#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-kwargs/autocorrect-kwargs.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a autocorrect-kwargs.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat autocorrect-kwargs.rb

rm autocorrect-kwargs.rb
rm "$tmp"
