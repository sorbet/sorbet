#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-array-compact/autocorrect-array-compact.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

# Run the autocorrect in place, on the temp file
$cwd/main/sorbet --silence-dev-message -a autocorrect-array-compact.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# cat the file to see the autocorrected output
cat autocorrect-array-compact.rb

rm autocorrect-array-compact.rb
rm "$tmp"
