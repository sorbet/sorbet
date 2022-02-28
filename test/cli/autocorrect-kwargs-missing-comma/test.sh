#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-kwargs-missing-comma/autocorrect-kwargs-missing-comma.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a autocorrect-kwargs-missing-comma.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat autocorrect-kwargs-missing-comma.rb

rm autocorrect-kwargs-missing-comma.rb
rm "$tmp"
