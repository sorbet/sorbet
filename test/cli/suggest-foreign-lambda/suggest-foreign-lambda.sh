#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/suggest-foreign-lambda/suggest-foreign-lambda.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a suggest-foreign-lambda.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat suggest-foreign-lambda.rb

rm suggest-foreign-lambda.rb
rm -r "$tmp"
